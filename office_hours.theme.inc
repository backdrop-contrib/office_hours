<?php
/**
 * @file
 * Themes the Office hours formatter and widget.
 *
 */

/**
 * Theme function for 'default' text field formatter.
 */
function office_hours_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  $settings = $display['settings'];
  $formatter = $display['type'];
  $element[0] = array(
    '#markup' => theme($display['type'] . '_formatter_default', array('element' => $items, 'field' => $instance)),
  );
  return $element;
}

/**
 * Theme function for 'default' text field formatter.
 */
function theme_office_hours_formatter_default($vars) {
  $output = '';
  $items = array();
  $first = variable_get('date_first_day', 0); 
  $field = field_info_field($vars['field']['field_name']);
  $element = $vars['element'];
  $weekdays = array(0 => t('Sunday'), 1 => t('Monday'), 2 => t('Tuesday'), 3 => t('Wednesday'), 4 => t('Thursday'), 5 => t('Friday'), 6 => t('Saturday') );

  foreach (element_children($element) as $key => $arraykey) {
    $item = $element[$arraykey];
    $day = (int)($item['day'] / 2); // Keys are 0+1 for sunday, 2+3 for monday, etc. Each day may have normal hours + extra hours.
    if (isset($day)) {
      $strhrs = _office_hours_mil_to_tf(check_plain($item['starthours']));  
      $endhrs = _office_hours_mil_to_tf(check_plain($item['endhours']));
      if ($field['settings']['hoursformat']) {
        $strhrs = _office_hours_convert_to_ampm($strhrs);
        $endhrs = _office_hours_convert_to_ampm($endhrs);
      }
      $items[$day][] = array('strhrs' => $strhrs, 'endhrs' => $endhrs) ; //we're aggregating hours for days together.
    }   
  }

  // add the closed days again, to 1) sort by first_day_of_week and 2) toggle show on/off
  foreach ($weekdays as $key => $day) {
    if (!array_key_exists($key, $items)) {
        $items[$key][]= array('closed' => 'closed'); //silly, but we need this as an array because we can't use a string offset later
    }
  }
  ksort($items);
  $items = date_week_days_ordered($items);
  $weekdays = date_week_days_ordered($weekdays);

  foreach ($items as $day => $hours) {
    $dayname = $weekdays[$day];
    $closed = '';
    $regular = '';
    $additional = '';
    if ( isset($hours[0]['closed']) ) {
      if ( !empty($field['settings']['showclosed']) ) { // don't output unnecessary fields
        $closed = '<span class="oh-display-hours">' . t('Closed') . '</span>';  
      }
    }
    else {
      $strhrs1 = $hours[0]['strhrs'];
      $endhrs1 = $hours[0]['endhrs'];
      $regular    = '<span class="oh-display-hours">' . $strhrs1 . ' - ' . $endhrs1 . '</span>';
      if (isset($hours[1])) {
        $strhrs2 = $hours[1]['strhrs'];
        $endhrs2 = $hours[1]['endhrs'];
        $additional = ' , <span class="oh-display-hours">' . $strhrs2 . ' - ' . $endhrs2 . '</span>';
      } 
    }

    $output_line = $closed . $regular . $additional ;  
    if (!empty($output_line)) {
      $output .= '<div class="oh-display">' . $dayname . ': ' . $output_line . '</div>';  
    }
  }
  return $output;
}

function theme_office_hours($vars) {
  return $vars['element']['#children'];
}

/**
 * Theme function for the office hours selector element.
 */
function theme_office_hours_select($vars) {
  return $vars['element']['#children'];
}

/**
 * copied from content module's theme_content_multiple_values- we're taking out the draggable feature.
 *
 * Theme the hour table form
 *
 */
function office_hours_field_multiple_value_form($variables) {
  // Use Field's theme function for non-office hours fields.
  if (empty($variables['element'][0]['#type']) || $variables['element'][0]['#type'] != 'office_hours') {
    return theme_field_multiple_value_form($variables);
  }

  // The following is a copy from standard theme_field_multiple_value_form()
  // http://api.drupal.org/api/drupal/modules%21field%21field.form.inc/function/theme_field_multiple_value_form/7
  // Modifications for the Office Hours form are commented out.
  // We've removed the columns 'order' and 'drag&drop', and do not add the drap&drop JS.

  $element = $variables['element'];
  $output = '';
  if ($element['#cardinality'] > 1 || $element['#cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
    $table_id = drupal_html_id($element['#field_name'] . '_values');
    $order_class = $element['#field_name'] . '-delta-order';
    $required = !empty($element['#required']) ? theme('form_required_marker', $variables) : '';

    $header = array(
      array(
        'data' => '<label>' . t('!title: !required', array('!title' => $element['#title'], '!required' => $required)) . "</label>",
        'colspan' => 2,
        'class' => array('field-label'),
      ),
//      t('Order'),
    );
    $rows = array();

    // Sort items according to '_weight' (needed when the form comes back after
    // preview or failed validation)
    $items = array();
    foreach (element_children($element) as $key) {
      if ($key === 'add_more') {
        $add_more_button = &$element[$key];
      }
      else {
        $items[] = &$element[$key];
      }
    }
    usort($items, '_field_sort_items_value_helper');

    $items = _office_hours_arrange_day($items); //this calls the function that arranges the first day of the week.

    // Add the items as table rows.
    foreach ($items as $key => $item) {
      $item['_weight']['#attributes']['class'] = array($order_class);
      $delta_element = drupal_render($item['_weight']);
      $cells = array(
//        array('data' => '', 'class' => array('field-multiple-drag')),
        drupal_render($item),
//        array('data' => $delta_element, 'class' => array('delta-order')),
      );
      $rows[] = array(
        'data' => $cells,
//        'class' => array('draggable'),
      );
    }

    $output = '<div class="form-item">';
    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => $table_id, 'class' => array('field-multiple-table'))));
    $output .= $element['#description'] ? '<div class="description">' . $element['#description'] . '</div>' : '';
    $output .= '<div class="clearfix">' . drupal_render($add_more_button) . '</div>';
    $output .= '</div>';

//    drupal_add_tabledrag($table_id, 'order', 'sibling', $order_class);
  }
  else {
    foreach (element_children($element) as $key) {
      $output .= drupal_render($element[$key]);
    }
  }

  return $output;
}
