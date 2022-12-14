<?php

/**
 * @file
 * Implements the office_hours Field.
 */

/**
 * Implements hook_field_info().().
 */
function office_hours_field_info() {
  return array(
    'office_hours' => array(
      'label' => t('Office hours'),
      'description' => t("This field stores weekly 'office hours' or 'opening hours' in the database."),
      'settings' => array(
        'comment' => 0,
        'date_first_day' => 0,
        'hoursformat' => 0,
        'granularity' => 60,
        'limitstart' => '',
        'limitend' => '',
        'valhrs' => 0,
        // Mar-2013: Conversion from (old) checkbox "Add more hours" to (new) selectlist 'cardinality'.
        // @todo for 'multiple blocks per day': create hook_update_N().
        // 'addhrs' => 1,
        'cardinality' => 2,
      ),
      'default_widget' => 'office_hours',
      'default_formatter' => 'office_hours',
      // Integrate with the Entity Metadata module.
      'property_type' => 'office_hours',
      'property_callbacks' => array('office_hours_property_info_callback'),
    ),
  );
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function office_hours_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];

  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);

  $property['property info']['day'] = array(
    'type' => 'integer',
    'label' => t('Day'),
    'description' => "Stores the day of the week's numeric representation (0-6)",
    'setter callback' => 'entity_property_verbatim_set',
    'default' => 0,
  );
  $property['property info']['starthours'] = array(
    'type' => 'integer',
    'label' => t('Start Hours'),
    'description' => 'Stores the start hours value',
    'setter callback' => 'entity_property_verbatim_set',
    'default' => 0,
  );
  $property['property info']['endhours'] = array(
    'type' => 'integer',
    'label' => t('End Hours'),
    'description' => 'Stores the end hours value',
    'setter callback' => 'entity_property_verbatim_set',
    'default' => 0,
  );
  $property['property info']['comment'] = array(
    'type' => 'varchar',
    'label' => t('Comment'),
    'description' => 'Stores the comment',
    'setter callback' => 'entity_property_verbatim_set',
    'default' => 0,
  );
}

/**
 * Implements hook_form_FORM_ID_alter().().
 *
 * Changes the hook_field_settings_form.
 */
function office_hours_form_field_ui_field_edit_form_alter(&$form, &$form_state, $form_id) {
  if ($form['#field']['type'] == 'office_hours') {
    $description = t("This is unlimited by this field's nature. See below for limiting the number of blocks per day.");
    $form['field']['cardinality'] = array(
      '#type' => 'select',
      '#title' => t('Number of values'),
      // '#options' => backdrop_map_assoc(array(14)) + array(FIELD_CARDINALITY_UNLIMITED => t('unlimited')) + backdrop_map_assoc(range(1, 10)),
      // '#options' => backdrop_map_assoc(array(14)),
      // '#default_value' => 14,
      '#options' => array(FIELD_CARDINALITY_UNLIMITED => t('unlimited')),
      '#default_value' => FIELD_CARDINALITY_UNLIMITED,
      '#description' => $description,
    );
  }
}

/**
 * Implements hook_field_settings_form().().
 *
 * Handle the global parameters for a field.
 */
function office_hours_field_settings_form($field, $instance, $has_data) {
  $settings = $field['settings'];

  // Adapt field settings to this special case.
  $my_settings = $field['settings'];
  $my_settings['valhrs'] = FALSE;
  $my_settings['limitstart'] = '';
  $my_settings['limitend'] = '';
  $my_settings['hoursformat'] = 'H';
  $hours = _office_hours_show_ampm(_office_hours_field_widget_hours($my_settings));

  $form = array();
  $form['#element_validate'] = array('_office_hours_field_settings_form_validate');
  $description = t('The maximum number of blocks, that are allowed per day.');
  $description .= '<br/><strong>' . t('Warning! Lowering this setting after data has been created could result in the loss of data!') . '</strong><br/>';
  $description .= t('Be careful when using more then 2 blocks per day, since not all external services (like Google Places) support this.');
  $form['cardinality'] = array(
    '#type' => 'select',
    '#title' => t('Number of blocks'),
    // @todo for 'multiple blocks per day': add support for FIELD_CARDINALITY_UNLIMITED.
    // '#options' => array(FIELD_CARDINALITY_UNLIMITED => t('Unlimited')) + backdrop_map_assoc(range(1, 10)),
    // '#options' => backdrop_map_assoc(range(1, 2)),
    '#options' => backdrop_map_assoc(range(1, 12)),
    '#default_value' => $settings['cardinality'],
    '#description' => $description,
    // '#disabled' => $has_data,
  );

  // Mar-2013: Convert (old) checkbox "Add more hours" to (new) selectlist.
  // @todo for 'multiple blocks per day': remove, after create hook_update_N().
  if (isset($settings['addhrs'])) {
    $form['addhrs'] = array(
      // '#type' => 'checkbox',
      '#type' => 'hidden',
      '#title' => t('Display the "Add more hours" link'),
      '#required' => FALSE,
      '#default_value' => $settings['addhrs'],
      '#description' => t('Make it possible to use 2 hour block for each day instead of one.'),
    );
    if (!is_null($settings['addhrs']) && $settings['addhrs'] == 0 && $settings['cardinality'] == 2) {
      $form['addhrs']['#default_value'] = NULL;
      $form['cardinality']['#default_value'] = 1;
    }
  }

  // First day of week, copied from system.variable.inc.
  $form['date_first_day'] = array(
    '#type' => 'select',
    '#options' => date_week_days(TRUE),
    '#title' => t('First day of week'),
    '#default_value' => $settings['date_first_day'],
    '#description' => t('First day in the widget. (A separate setting exists for the formatter.)'),
  );
  $form['hoursformat'] = array(
    '#type' => 'select',
    '#title' => t('Hours format'),
    '#options' => array(
      2 => t('24 hrs') . ' (09:00)',
      0 => t('24 hrs') . ' (9:00)',
      1 => t('12 hrs') . ' (9:00 am)',
      3 => t('12 hrs') . ' (9:00 a.m.)',
    ),
    '#default_value' => $settings['hoursformat'],
    '#required' => FALSE,
    '#description' => t('Format of the clock in the widget.'),
  );
  $form['granularity'] = array(
    '#type' => 'select',
    '#title' => t('Granularity of time'),
    '#options' => array(
      '60' => t('hours'),
      '30' => t('half hours'),
      '15' => t('quarter hours'),
      '5' => t('5 minute intervals'),
      '1' => t('minutes'),
    ),
    '#default_value' => $settings['granularity'],
    '#required' => FALSE,
    '#description' => t('Restrict the input to fixed fractions of an hour.'),
  );
  $form['comment'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow a comment per time slot'),
    '#required' => FALSE,
    '#default_value' => $settings['comment'],
    // '#description' => t('Allow a comment per time slot.'),
  );
  $form['valhrs'] = array(
    '#type' => 'checkbox',
    '#title' => t('Validate hours'),
    '#required' => FALSE,
    '#default_value' => $settings['valhrs'],
    '#description' => t('Assure that endhours are later then starthours. Please note that this will work as long as the opening hours are not through midnight.'),
  );
  $form['limitstart'] = array(
    '#type' => 'select',
    '#title' => t('Limit widget hours - from'),
    '#description' => t('Restrict the hours available - select options will start from this hour.'),
    '#default_value' => $settings['limitstart'],
    '#options' => $hours,
  );
  $form['limitend'] = array(
    '#type' => 'select',
    '#title' => t('Limit widget hours - until'),
    '#description' => t('Restrict the hours available - select options will end at this hour.'),
    '#default_value' => $settings['limitend'],
    '#options' => $hours,
  );

  /*
   // @Todo: activate this, to Show $form['limitstart'] and $form['limitend'] using the normal widget.
   $form_state = NULL;
   $default_value_field = $field;
   $default_value_field['settings']['granularity'] = '60';
   $langcode = 'und';
   $items = array();
   $delta = 0;
   $element = NULL;

   $elements = office_hours_field_widget_form($form, $form_state, $default_value_field, $instance, $langcode, $items, $delta, $element);

   $form['limits'] = $elements[0];
   $form['limits']['#dayname'] = 'Limit widget hours';
   $form['limits']['#description'] = t('Restrict the hours available - select options will be between these hours.');
   $form['limits']['#default_value'] = array(
   'day' => 0,
   'daydelta' => 0,
   'starthours' => $settings['limitstart'],
   'endhours' => $settings['limitend'],
   );
   */

  return $form;
}

/**
 * Implements the #element_validate callback for myfield_field_settings_form().
 *
 * Verifies the office hours limits.
 * "Please note that this will work as long as the opening hours are not through midnight."
 */
function _office_hours_field_settings_form_validate($element, &$form_state) {
  if ($element['limitstart']['#value'] > $element['limitend']['#value']) {
    form_error($element['limitstart'], t('%start is later then %end.', array('%start' => $element['limitstart']['#title'], '%end' => $element['limitend']['#title'])));
  }
}

/**
 * Implements hook_field_is_empty().().
 */
function office_hours_field_is_empty($item, $field) {
  if ($item['starthours'] == '' || $item['endhours'] == '' || !isset($item['day']) || $item['day'] === '') {
    return TRUE;
  }
  return FALSE;
}

/**
 * Checks if an instance of the office hours field type is empty.
 *
 * @param array $items
 *   An array of $entity->{$field['field_name']}[$langcode]
 *
 * @return boolean
 *   Whether or not the instance is considered empty.
 */
function office_hours_instance_is_empty($items) {
  foreach ($items as $delta => $item) {
    if (!office_hours_field_is_empty($item, 'office_hours_select')) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Implements hook_field_load(), hook_field_presave().
 * NB. Do NOT use these hooks. On the field_settings_form, we have a multi-value default value.
 * hook_field_load() and hook_field_presave() are NOT called on this page, destroying the data.
 * This hook is not called in the widget settings page. See http://drupal.org/node/1944678 .
 */
// function office_hours_field_load($entity_type, $entities, $field, $instances, $langcode, &$items, $age) {
// }
// function office_hours_field_presave($entity_type, $entity, $field, $instance, $langcode, &$items) {
// }

/**
 * Implements hook_field_validate().().
 * Is called a.o. when saving the content type field (default value) and content.
 */
function office_hours_field_validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {
  if (!empty($entity_type) && $instance['required']) {
    if (empty($items) || office_hours_instance_is_empty($items)) {
      $errors[$field['field_name']][$langcode][0][] = array(
        'error' => 'office_hours_required',
        'message' => t('%name field is required. Please fill in one or more of the hour blocks.', array('%name' => $instance['label'])),
      );
    }
  }
}
