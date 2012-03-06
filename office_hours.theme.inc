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
//  '#markup' => theme($display['type'] . '_formatter_default', array('element' => $items, 'field' => $instance)),
    '#markup' => theme($field['type'] . '_formatter_default', array('element' => $items, 'field' => $instance, 'display' => $display)),
  );
  return $element;
}

/**
 * Theme function for field formatter.
 * Adapted for 2 types:
 * - default, 'normal', tabular format
 * - inline format, adapted for use in interfaces such as Google Places.
 * This is put in 1 function, since there is a lot of pre-processing before the actual formatting,
 * So you'll find 2 parts: part1 = Preprocessing, part2 = Formatting, depending on the choosen formatter.
 * @TODO: use e.g. strtotime() / Date_module http://php.net/manual/en/function.date.php functions for proper time formatting.
 */
function theme_office_hours_formatter_default($vars) {
  $items = array();
  $field = field_info_field($vars['field']['field_name']);
  $element = $vars['element'];
  $display = $vars['display'];
  $settings = $vars['display']['settings'];
  $formatter = $vars['display']['type'];

  switch ($settings['daysformat']) {
    case 'long':
      $weekdays = date_week_days(TRUE);
      break;
    case 'short':
      $weekdays = date_week_days_abbr(TRUE, TRUE, 3); // Abbreviate up to 3 characters
      break;
    case 'number':
      $weekdays = array(0 => 1, 1 => 2, 2 => 3, 3 => 4, 4 => 5, 5 => 6, 6 => 7 );
      break;
    case 'none':
      $weekdays = array(0 => '', 1 => '', 2 => '', 3 => '', 4 => '', 5 => '', 6 => '' );
      $settings['separator_day_hours'] = '';  // override user's setting if daysformat = 'none'. (Should we?)
      break;
  }

  // Format the 'hours'.
  foreach (element_children($element) as $key => $arraykey) {
    $item = $element[$arraykey];
    $day = (int)($item['day'] / 2); // Keys are 0+1 for sunday, 2+3 for monday, etc. Each day may have normal hours + extra hours.
    if (isset($day)) {
      // @TODO: use e.g. strtotime() / Date_module http://php.net/manual/en/function.date.php functions for proper time formatting.
      $strhrs = _office_hours_mil_to_tf(check_plain($item['starthours']));
      $endhrs = _office_hours_mil_to_tf(check_plain($item['endhours']));
      if ($field['settings']['hoursformat']) {
        $strhrs = _office_hours_convert_to_ampm($strhrs);
        $endhrs = _office_hours_convert_to_ampm($endhrs);
      }
      $items[$day][] = array('strhrs' => $strhrs, 'endhrs' => $endhrs) ; // We're aggregating hours for days together.
    }
  }

  // add the closed days again, to 1) sort by first_day_of_week and 2) toggle show on/off
  foreach ($weekdays as $key => $day) {
    if (!array_key_exists($key, $items)) {
        $items[$key][]= array('closed' => 'closed'); //silly, but we need this as an array because we can't use a string offset later
    }
  }

  // Sort the days of the week again, because sorting is lost by adding the closed days.
  // Do not sort by first_day_of_week for certain types: 'number'/'none' are more frequently used by machines/interfaces.
  ksort($items);
  switch ($settings['daysformat']) {
    case 'long':
    case 'short':
      $items = date_week_days_ordered($items);
      $weekdays = date_week_days_ordered($weekdays);
      break;
    case 'number':
    case 'none':
      break;
  }

  $output = '';
  $values = array();
  foreach ($items as $day => $hours) {

    $closed = '';
    $regular = '';
    $additional = '';
    if ( isset($hours[0]['closed']) ) {
      if ( !empty($settings['showclosed']) ) { 
        // Format the empty day with a text like 'Closed' or '00:00-00:00'
        $closed = check_plain( t($settings['closedformat'] ) );
      }
      else {
        // Don't output unnecessary fields.
      }
    }
    else {
      $strhrs1 = $hours[0]['strhrs'];
      $endhrs1 = $hours[0]['endhrs'];
      if ( isset($hours[1] ) ) {
        if (!$settings['compress']) {
          $strhrs2 = $hours[1]['strhrs'];
          $endhrs2 = $hours[1]['endhrs'];
          $additional = $settings['separator_more_hours'] . $strhrs2 . $settings['separator_hours_hours'] . $endhrs2;
        }
        else {
          // Override endhours of morning with endhours of evening.
          $endhrs1 = $hours[1]['endhrs'];
        }
      }
      $regular = $strhrs1 . $settings['separator_hours_hours'] . $endhrs1;
    }

    $output_dayname = $weekdays[$day];

    switch ($formatter) {
      case 'office_hours':
        $maxdaylen = max( 3, max(array_map('strlen', $weekdays)) * 0.70) ;  // Make sure hours are aligned vertically. Should go in css.

        $output_hours = $closed . $regular . $additional;
        if (!empty($output_hours)) {
          $output .= '<div class="oh-display" style="white-space: nowrap; ">'
                   .   '<div class="oh-display-dayname" style="display: inline-block; width: ' . $maxdaylen . 'em; ">'
                   .     $output_dayname
                   .     $settings['separator_day_hours']
                   .   '</div>'
                   .   '<span class="oh-display-hours" style="white-space: nowrap; display: inline;">'
                   .     $closed . $regular . $additional
                   .   '</span>'
                   . '</div>';
        }
        break;

      case 'office_hours_inline':
        $output_hours = $closed . $regular . $additional;
        if (!empty($output_hours)) {
          $output .= (!empty($output)) ? $settings['separator_days'] : '';

          $output .= $output_dayname
                   . $settings['separator_day_hours']
                   . $closed . $regular . $additional;
        }

      break;
    }  // endswitch.
  }  // endforeach.

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
  $element = $variables['element'];
  $output = '';

  if ($element['#cardinality'] > 1 || $element['#cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
    $table_id = drupal_html_id($element['#field_name'] . '_values');
    $order_class = $element['#field_name'] . '-delta-order';
    $required = !empty($element['#required']) ? '<span class="form-required" title="' . t('This field is required. ') . '">*</span>' : '';

    $header = array(
      array(
        'data' => '<label>' . t('!title: !required', array('!title' => $element['#title'], '!required' => $required)) . "</label>",
        'colspan' => 2,
        'class' => array('field-label'),
      ),
      t('Order'),
    );
    $rows = array();
    $items = array();
    // Sort items according to '_weight' (needed when the form comes back after
    // preview or failed validation)
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
        array('data' => '', 'class' => array('field-multiple-drag')),
        drupal_render($item),
        array('data' => $delta_element, 'class' => array('delta-order')),
      );
      $rows[] = array(
        'data' => $cells,
        //'class' => array('draggable'),
      );
    }

    $output = '<div class="form-item">';
    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => $table_id, 'class' => array('field-multiple-table'))));
    $output .= $element['#description'] ? '<div class="description">' . $element['#description'] . '</div>' : '';
    $output .= '<div class="clearfix">' . drupal_render($add_more_button) . '</div>';
    $output .= '</div>';

    drupal_add_tabledrag($table_id, 'order', 'sibling', $order_class);
  }
  else {
    foreach (element_children($element) as $key => $value) {
      //$output .= office_hours_field_multiple_value_form(array('element' => $element[$value]));
      $output .= drupal_render($element[$key]);
    }
  }
  return $output;
}
