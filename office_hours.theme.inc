<?php

/**
 * Theme function for 'default' text field formatter.
 */

function office_hours_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
	$element = array();
	$settings = $display['settings'];
	$formatter = $display['type'];
	$element[0] = array(
		'#markup' => theme($display['type'].'_formatter_default', array('element' => $items, 'field' => $instance)),
		);
	return $element;
}

/**
 * Theme function for 'default' text field formatter.
 */
function theme_office_hours_formatter_default($vars) {
  $items = array();
  $element = $vars['element'];
  $first = variable_get('date_first_day',0); 
  $field = field_info_field($vars['field']['field_name']);
  $output = '';
  $days = array(0 => t('Sunday'), 1 => t('Monday'), 2 => t('Tuesday'),3 => t('Wednesday'), 4 => t('Thursday'), 5 => t('Friday'), 6 => t('Saturday') );
  foreach (element_children($element) as $key => $arraykey) {
    $item = $element[$arraykey];
    $day = $days[(int)($element[$arraykey]['day']/2)];
    if (isset($day)) {
      $endhrs = _office_hours_mil_to_tf(check_plain($item['endhours']));
      $strhrs = _office_hours_mil_to_tf(check_plain($item['starthours']));  
      if ($field['settings']['hoursformat']) {
        $endhrs = _office_hours_convert_to_ampm($endhrs);
        $strhrs = _office_hours_convert_to_ampm($strhrs);
      }
      $items[$day][] = array('strhrs' => $strhrs,'endhrs' => $endhrs) ;//we're aggregating hours for days together. 
    }   
  }
  
  $keys = array_keys($items);
  if ((!empty($keys)) && $keys[0] != $days[$first] && in_array(t($days[$first]), $items) )
   while ($keys[0] != $days[$first] && count($keys)>1):   
    $shifted = array_shift($items);
    $items[$keys[0]] = $shifted;
    $keys = array_keys($items);
  endwhile;
  }
  
  foreach ($items as $day => $val) {
    $strhrs = $val[0]['strhrs'];
    $endhrs = $val[0]['endhrs'];
	if (isset($val[1])) {
    $strhrs2 = $val[1]['strhrs'];
    $endhrs2 = $val[1]['endhrs'];
    $additional =  ' <span class="oh-display-hours">, '.$strhrs2.' - '.$endhrs2.'</span>';
	} else $additional = '';
    $output .= '<div class="oh-display">'.$day.': <span class="oh-display-hours">'.$strhrs.' - '.$endhrs.'</span>'.$additional.'</div>';  
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
function theme_office_hours_multiple_values($vars) {
 $element = $vars['element'];
 $output = '';
 if (isset($element['#cardinality']) and  $element['#cardinality'] >= 1) {
    $table_id = $element['#field_name'] .'_values';
    $order_class = $element['#field_name'] .'-delta-order';
    $required = !empty($element['#required']) ? '<span class="form-required" title="'. t('This field is required.') .'">*</span>' : '';

    $header = array(
      array(
        'data' => t('!title: !required', array('!title' => $element['#title'], '!required' => $required)),
        'colspan' => 2
      ),
    );
    $rows = array();

    // Sort items according to '_weight' (needed when the form comes back after
    // preview or failed validation)
    $items = array();
	foreach (element_children($element) as $key=>$value) {
      if ($key !== $element['#field_name'] .'_add_more') {
         $items[] = &$element[$key];
      }
    }
  // dpm($items);
    $items = _office_hours_arrange_day($items); //this calls the function that arranges the first day of the week.
    // Add the items as table rows.
    foreach ($items as $key => $item) {
      //$item['#type'] = 'office_hours';
	  $delta_element = drupal_render($item['_weight']);
      if (!($key % 2)) { //this is an even row, start a new row array_keys
        $cells = array (
         drupal_render($item),
        );
     }
      else { //this is an odd row
        $cells[] = drupal_render($item);  //we've add the second cell;
        $rows[] = array(
        'data' => $cells,
        );
      }
    }

    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'id' => $table_id, 'class' => 'office_hours_table content-multiple-table'));
    $output .= $element['#description'] ? '<div class="description">'. $element['#description'] .'</div>' : '';
  //  $output .= drupal_render($element[$element['#field_name'] .'_add_more']);

    //drupal_add_tabledrag($table_id, 'order', 'sibling', $order_class);
  }
  else {
    foreach (element_children($element) as $key=>$value) {
      $output .= theme_office_hours_multiple_values(array('element' => $element[$value]));
    }
  }
  
  return $output;
  
}
