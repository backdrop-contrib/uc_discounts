<?php

/**
 * @file
 * Admin forms and functions for uc_discounts module.
 */

/**
 * Generates admin page listing all saved discounts.
 */
function uc_discounts_admin_discounts_list() {
  $search_params = array(
    'name' => arg(4),
  );

  $header = array(
    array(
      'data' => t('Active'),
      'field' => 'is_active',
    ),
    array(
      'data' => t('Name'),
      'field' => 'name',
    ),
    array(
      'data' => t('Short Description'),
      'field' => 'short_description',
    ),
    array(
      'data' => t('Qualifying Type'),
      'field' => 'qualifying_type',
    ),
    array(
      'data' => t('Type'),
      'field' => 'discount_type',
    ),
    array(
      'data' => t('Amount'),
      'field' => 'discount_amount',
    ),
    array(
      'data' => t('Weight'),
      'field' => 'weight',
    ),
    array(
      'data' => t('Created At'),
      'field' => 'insert_timestamp',
      'sort' => 'desc',
    ),
    array(
      'data' => t('Expires At'),
      'field' => 'expiration',
    ),
    array('data' => t('Operations')),
  );

  $select = db_select('uc_discounts', 'd')
    ->fields('d')
    ->extend('PagerDefault')
    ->limit(50)
    ->extend('TableSort')
    ->orderByHeader($header);

  if (!empty($search_params['name'])) {
    $name = '%' . $search_params['name'] . '%';
    $select->condition(db_or()
        ->condition('d.name', $name, 'LIKE')
        ->condition('d.description', $name, 'LIKE')
        ->condition('d.short_description', $name, 'LIKE'));
  }

  $result = $select->execute()->fetchAll();

  $rows = array();
  foreach ($result as $discount) {
    //$total_use_count = (is_numeric($discount->total_use_count)) ? $discount->total_use_count : 0;
    //$total_times_applied = (is_numeric($discount->total_times_applied)) ? $discount->total_times_applied : 0;
    $operations = array(
      l(t('usage'), 'admin/reports/uc_discounts/discount/' . $discount->discount_id),
      l(t('edit'), 'admin/store/uc_discounts/edit/' . $discount->discount_id),
      l(t('copy'), 'admin/store/uc_discounts/copy/' . $discount->discount_id),
      l(t('delete'), 'admin/store/uc_discounts/delete/' . $discount->discount_id),
    );

    $rows[] = array(
      'data' => array(
        array(
          'data' => ($discount->is_active ? '&#10003;' : 'X'),
          'class' => array(($discount->is_active ? 'is_active' : 'is_inactive')),
        ),
        $discount->name,
        $discount->short_description,
        uc_discounts_qualifying_type_name($discount->qualifying_type),
        uc_discounts_discount_type_name($discount->discount_type),
        array(
          'data' => uc_discounts_discount_amount_formatted($discount),
          'class' => array('numeric'),
        ),
        array(
          'data' => $discount->weight,
          'class' => array('numeric'),
        ),
        array(
          'data' => format_date($discount->insert_timestamp, 'short'),
          'class' => array('date'),
        ),
        array(
          'data' => ($discount->has_expiration ? format_date($discount->expiration, 'short') : t('N/A')),
          'class' => array('date'),
        ),
        array(
          'data' => implode(' ', $operations),
          'class' => array('operations'),
        ),
      ),
    );
  }

  if (empty($rows)) {
    $rows[] = array(
      'data' => array(
        array(
          'data' => t('No discounts.'),
          'colspan' => count($header),
        ),
      ),
    );
  }

  $output['content'] = array(
    'search_form' => drupal_get_form('uc_discounts_admin_discounts_search_form', $search_params),
    'list_table' => array(
      '#markup' => theme('table', array(
        'header' => $header,
        'rows' => $rows,
        'attributes' => array(
          'class' => array('uc-discounts')
        )
      ))
    ),
    'pager' => array('#theme' => 'pager', '#weight' => 5),
  );

  return $output;
}

/**
 * Creates an admin form for searching discounts by name.
 *
 * @ingroup forms
 * @see uc_discounts_admin_discounts_list()
 * @see uc_discounts_admin_search_discounts_form_submit()
 */
function uc_discounts_admin_discounts_search_form($form, &$form_state, $values) {
  $form = array();

  $form['search'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search discounts'),
    '#collapsible' => TRUE,
    '#collapsed' => empty($values['name']),
  );

  $form['search']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Description/Name'),
    '#default_value' => $values['name'],
  );

  $form['search']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );

  return $form;
}

/**
 * Form submit handler for uc_discounts_admin_search_discounts_form().
 *
 * @ingroup forms
 * @see uc_discounts_admin_search_discounts_form()
 */
function uc_discounts_admin_discounts_search_form_submit($form, &$form_state) {
  $args = array(
    $form_state['values']['name'],
  );
  $form_state['redirect'] = array('admin/store/uc_discounts/list/' . implode('/', $args));
}

/**
 * Generates admin form to create or edit a discount.
 *
 * @ingroup forms
 * @see uc_discounts_admin_discount_edit_form_submit()
 * @see uc_discounts_admin_discount_edit_form_validate()
 */
function uc_discounts_admin_discount_edit_form($form, &$form_state, $discount_id = 0) {
  $form = array();
  $form_submitted = $form_state['submitted'];
  $is_edit = $discount_id != 0;

  if (!$form_submitted && $is_edit) {
    $form_state['values'] = (array) uc_discounts_load($discount_id);
  }

  $form['discount_id'] = array(
    '#type' => 'hidden',
    '#value' => isset($form_state['values']['discount_id']) ? $form_state['values']['discount_id'] : '',
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Shown in admin reports and logs.'),
    '#default_value' => isset($form_state['values']['name']) ? $form_state['values']['name'] : '',
    '#size' => 50,
    '#required' => TRUE,
  );

  $form['short_description'] = array(
    '#type' => 'textfield',
    '#title' => t('Short Description'),
    '#description' => t('Description displayed to user during checkout and in order review.'),
    '#default_value' => isset($form_state['values']['short_description']) ? $form_state['values']['short_description'] : '',
    '#size' => 75,
    '#required' => TRUE,
  );

  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#description' => t('Internal description for discount administrators.'),
    '#default_value' => isset($form_state['values']['description']) ? $form_state['values']['description'] : '',
    '#size' => 75,
  );

  $is_active = isset($form_state['values']['is_active']) ? $form_state['values']['is_active'] : FALSE;
  $form['is_active'] = array(
    '#type' => 'checkbox',
    '#title' => t('Is active'),
    '#description' => t('Is this discount active for use on the site?'),
    '#default_value' => $is_active,
  );

  $form['can_be_combined_with_other_discounts'] = array(
    '#type' => 'checkbox',
    '#title' => t('Can be combined with other discounts'),
    '#default_value' => isset($form_state['values']['can_be_combined_with_other_discounts']) ? $form_state['values']['can_be_combined_with_other_discounts'] : FALSE,
  );

  $has_activation = isset($form_state['values']['has_activation']) ? $form_state['values']['has_activation'] : FALSE;
  $form['has_activation'] = array(
    '#type' => 'checkbox',
    '#title' => t('Discount activates'),
    '#description' => t('Whether or not this discount rule will only become active after a given date.'),
    '#default_value' => $has_activation,
  );

  $activates_on = isset($form_state['values']['activates_on']) ? $form_state['values']['activates_on'] : REQUEST_TIME;
  $form['activates_on'] = array(
    '#type' => 'date_popup',
    '#date_type' => 'DATE_UNIX',
    '#title' => t('Discount activation date'),
    '#description' => t('Date and time when the discount will become active.  Note that if the %is_active field is not checked, this will be ignored.', array('%is_active' => t('Is active'))),
    '#default_value' => date('Y-m-d H:i:s', $activates_on),
    '#states' => array(
      'visible' => array(
        'input[name="has_activation"]' => array('checked' => TRUE),
      ),
      'required' => array(
        'input[name="has_activation"]' => array('checked' => TRUE),
      ),
    ),
  );

  $has_expiration = isset($form_state['values']['has_expiration']) ? $form_state['values']['has_expiration'] : FALSE;
  $form['has_expiration'] = array(
    '#type' => 'checkbox',
    '#title' => t('Discount expires'),
    '#description' => t('Whether or not this discount rule will expire on a given date.'),
    '#default_value' => $has_expiration,
  );

  $expiration = isset($form_state['values']['expiration']) ? $form_state['values']['expiration'] : REQUEST_TIME;
  $form['expiration'] = array(
    '#type' => 'date_popup',
    '#date_type' => 'DATE_UNIX',
    '#title' => t('Discount expiration'),
    '#description' => t('Date and time when the discount expires.'),
    '#default_value' => date('Y-m-d H:i:s', $expiration),
    '#states' => array(
      'visible' => array(
        'input[name="has_expiration"]' => array('checked' => TRUE),
      ),
      'required' => array(
        'input[name="has_expiration"]' => array('checked' => TRUE),
      ),
    ),
  );

  // Add custom weight range [-50, 50].
  $options = array();
  for ($i = -50; $i <= 50; $i++) {
    $options[$i] = $i;
  }
  $form['weight'] = array(
    '#type' => 'select',
    '#title' => t('Weight'),
    '#description' => t('Lighter discounts are applied to an order first. This value is unimportant if there are no discounts on discount line items.'),
    '#options' => $options,
    '#default_value' => isset($form_state['values']['weight']) ? $form_state['values']['weight'] : 0,
  );

  $form['qualifications'] = array(
    '#type' => 'fieldset',
    '#title' => t('Conditions of Qualification'),
    '#collapsible' => TRUE,
    '#description' => t('Carts qualify for a discount based upon these conditions.'),
  );

  $form['qualifications']['qualifying_type'] = array(
    '#type' => 'select',
    '#title' => t('Qualification type'),
    '#description' => t('The type of qualification used to determine if a cart qualifies for this discount.'),
    '#options' => uc_discounts_qualifying_type_options(),
    '#default_value' => isset($form_state['values']['qualifying_type']) ? $form_state['values']['qualifying_type'] : '',
  );

  $min_qual_amount_title = t('Minimum qualification amount');
  $form['qualifications']['qualifying_amount'] = array(
    '#type' => 'textfield',
    '#title' => $min_qual_amount_title,
    '#description' => t('The amount of qualification type required. E.g. 50 (for $50), 5 (for 5 items). Remember for a discount like "buy 4 get 1 free" the qualifying amount is "5". For a discount like "spend at least $25, get $5 off" the qualifying amount is "30".'),
    '#default_value' => isset($form_state['values']['qualifying_amount']) ? $form_state['values']['qualifying_amount'] : '',
    '#size' => 15,
    '#required' => TRUE,
  );

  $form['qualifications']['use_only_discounted_products_to_qualify'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use only %discounted_products to determine if %min_qual_amount_title is met.', array('%discounted_products' => t('Discounted Products'), '%min_qual_amount_title' => $min_qual_amount_title)),
    '#description' => t('If checked, uses only the products included by the %discounted_products input.  Otherwise it will be based off %required_products or all products in the cart.', array('%discounted_products' => t('Discounted Products'), '%required_products' => t('Required Products'))),
    '#default_value' => isset($form_state['values']['use_only_discounted_products_to_qualify']) ? $form_state['values']['use_only_discounted_products_to_qualify'] : FALSE,
  );

  $has_qualifying_amount_max = isset($form_state['values']['has_qualifying_amount_max']) ? $form_state['values']['has_qualifying_amount_max'] : FALSE;
  $form['qualifications']['has_qualifying_amount_max'] = array(
    '#type' => 'checkbox',
    '#title' => t('Has max qualifying amount.'),
    '#description' => t('Whether or not discount has a maximum amount above which the discount will not be applied.'),
    '#default_value' => $has_qualifying_amount_max,
  );

  $form['qualifications']['qualifying_amount_max'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum qualifying amount'),
    '#description' => t('Maximum amount to NOT exceed to qualify for the discount.  E.g. 50 (for $50), 5 (for 5 items).'),
    '#default_value' => isset($form_state['values']['qualifying_amount_max']) ? $form_state['values']['qualifying_amount_max'] : '',
    '#size' => 15,
    '#states' => array(
      'visible' => array(
        'input[name="has_qualifying_amount_max"]' => array('checked' => TRUE),
      ),
      'required' => array(
        'input[name="has_qualifying_amount_max"]' => array('checked' => TRUE),
      ),
    ),
  );

  $requires_code = isset($form_state['values']['requires_code']) ? $form_state['values']['requires_code'] : FALSE;
  $form['qualifications']['requires_code'] = array(
    '#type' => 'checkbox',
    '#title' => t('Require code to activate discount.'),
    '#default_value' => $requires_code,
  );

  // Determine codes default value.
  if (isset($form_state['values']['codes'])) {
    $codes_string = $form_state['values']['codes'];
  }
  else {
    $codes = array();
    if ($is_edit) {
      $codes = uc_discounts_get_codes_for_discount_id($discount_id);
    }
    $codes_string = uc_discounts_codes_to_str($codes);
  }
  $form['qualifications']['codes'] = array(
    '#type' => 'textarea',
    '#title' => t('Discount codes'),
    '#description' => t('Enter discount codes in box above, one code per line.  Spaces are permitted but may confuse consumers.  You may use the !link if you need to create a lot of unique codes.', array('!link' => l(t('discount code generator'), 'admin/store/uc_discounts/generate_codes/' . $discount_id))),
    '#default_value' => $codes_string,
    '#rows' => 5,
    '#states' => array(
      'visible' => array(
        'input[name="requires_code"]' => array('checked' => TRUE),
      ),
      'required' => array(
        'input[name="requires_code"]' => array('checked' => TRUE),
      ),
    ),
  );

  $has_role_filter = isset($form_state['values']['has_role_filter']) ? $form_state['values']['has_role_filter'] : FALSE;
  $form['qualifications']['has_role_filter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Qualification by Role(s).'),
    '#default_value' => $has_role_filter,
    '#attributes' => array('onchange' => "jQuery('#discount-role-ids-wrapper').toggle();"),
  );

  // Get current discount roles.
  $role_ids = null;
  if (isset($form_state['values']['role_ids'])) {
    $role_ids = $form_state['values']['role_ids'];
  }
  elseif ($is_edit) {
    $role_ids = uc_discounts_get_role_ids_for_discount_id($discount_id, FALSE);
  }

  // Create roles form element.
  $options = array();
  $result = db_query('SELECT rid, name
                     FROM {role}
                     ORDER BY rid');
  $options[UC_DISCOUNTS_OPTION_ALL_ROLES] = t('<All Roles>');
  foreach ($result as $row) {
    $options[$row->rid] = $row->name;
  }
  $form['qualifications']['role_ids'] = array(
    '#type' => 'select',
    '#title' => t('Roles'),
    '#description' => t('Select all roles that this discount applies to or &lt;All Roles&gt; to apply to all roles.'),
    '#options' => $options,
    '#default_value' => $role_ids,
    '#multiple' => TRUE,
    '#states' => array(
      'visible' => array(
        'input[name="has_role_filter"]' => array('checked' => TRUE),
      ),
      'required' => array(
        'input[name="has_role_filter"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['qualifications']['requires_single_product_to_qualify'] = array(
    '#type' => 'checkbox',
    '#title' => t('Require single product SKU to qualify.'),
    '#description' => t('Requires products with unique SKUs to meet the qualifying requirements. Otherwise, qualifying requirements can be met by a combination of different products.'),
    '#default_value' => isset($form_state['values']['requires_single_product_to_qualify']) ? $form_state['values']['requires_single_product_to_qualify'] : FALSE,
  );

  _uc_discounts_product_filter_form($form, $form_state, UC_DISCOUNTS_GROUPING_QUALIFICATION, $discount_id);

  $form['application'] = array(
    '#type' => 'fieldset',
    '#title' => t('Discount Application'),
    '#description' => t('Details of the discount to apply'),
    '#collapsible' => TRUE,
  );

  $form['application']['discount_type'] = array(
    '#type' => 'select',
    '#title' => t('Discount type'),
    '#description' => t('Type of discount to apply.'),
    '#options' => uc_discounts_discount_type_options(),
    '#default_value' => isset($form_state['values']['discount_type']) ? $form_state['values']['discount_type'] : '',
  );

  $form['application']['discount_amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Discount amount'),
    '#description' => t('The amount of discount.  E.g. 50 (for $50), 5 (for 5 items), or 0.05 (for 5%)'),
    '#default_value' => isset($form_state['values']['discount_amount']) ? $form_state['values']['discount_amount'] : '',
    '#size' => 15,
    '#required' => TRUE,
  );

  _uc_discounts_product_filter_form($form, $form_state, UC_DISCOUNTS_GROUPING_APPLICATION, $discount_id);

  $form['application']['add_to_cart'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add "Discounted products" to cart.'),
    '#description' => t('Ensure that the selected items are added to the cart if not already present. If already present, update the quantity appropriately. You must set at least one "Required product" above and a "Discount type" of "Free items" in order for this to be effective. Does not work with discounts that require a Code.'),
    '#default_value' => isset($form_state['values']['add_to_cart']) ? $form_state['values']['add_to_cart'] : FALSE,
  );

  $form['application']['max_times_applied'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum applications'),
    '#description' => t('How many times can this discount be applied in a single cart (0 for unlimited)?  Does not apply to "Percentage off" or "Fixed amount off" discounts.'),
    '#default_value' => isset($form_state['values']['max_times_applied']) ? $form_state['values']['max_times_applied'] : 0,
    '#size' => 7,
    '#required' => TRUE,
  );

  $form['application']['limit_max_times_applied'] = array(
    '#type' => 'checkbox',
    '#title' => t('Further limit maximum applications to number of Required Products in cart.'),
    '#description' => t('Allows for a discount like "Get half off X for each Y that is purchased."'),
    '#default_value' => isset($form_state['values']['limit_max_times_applied']) ? $form_state['values']['limit_max_times_applied'] : FALSE,
    '#size' => 7,
  );

  $form['application']['max_uses'] = array(
    '#type' => 'textfield',
    '#title' => t('Max uses'),
    '#description' => t('Number of times this discount can be applied (0 for unlimited).'),
    '#default_value' => isset($form_state['values']['max_uses']) ? $form_state['values']['max_uses'] : 0,
    '#size' => 7,
    '#required' => TRUE,
  );

  $form['application']['max_uses_per_user'] = array(
    '#type' => 'textfield',
    '#title' => t('Max uses per user'),
    '#description' => t('Number of times this discount can be applied to a particular user (0 for unlimited).'),
    '#default_value' => isset($form_state['values']['max_uses_per_user']) ? $form_state['values']['max_uses_per_user'] : 0,
    '#size' => 7,
    '#required' => TRUE,
  );

  $form['application']['max_uses_per_code'] = array(
    '#type' => 'textfield',
    '#title' => t('Max uses per code'),
    '#description' => t('Number of times this discount can be applied to a particular code (0 for unlimited).  Note: "Max uses" (if set) still applies as overall maximum number of uses for this discount.'),
    '#default_value' => isset($form_state['values']['max_uses_per_code']) ? $form_state['values']['max_uses_per_code'] : 0,
    '#size' => 7,
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Helper function that creates a series of dropdowns for selecting a product
 * via product_id, sku, class, term, or author.
 */
function _uc_discounts_product_filter_form(&$form, &$form_state, $grouping, $discount_id) {
  $fieldset_name = $grouping == UC_DISCOUNTS_GROUPING_APPLICATION ? 'application' : 'qualifications';

  $form[$fieldset_name]['filter'] = array(
    '#type' => 'fieldset',
    '#title' => $grouping == UC_DISCOUNTS_GROUPING_APPLICATION ? t('Discounted products') : t('Required products'),
    '#description' => $grouping == UC_DISCOUNTS_GROUPING_APPLICATION ? t('Select the products which will be discounted.') : t('If products are selected, at least one of them will be required.'),
  );

  $options = array();
  if ($grouping == UC_DISCOUNTS_GROUPING_QUALIFICATION) {
    $options[UC_DISCOUNTS_FILTER_TYPE_NONE] = t('No specific product');
    $filter_type = isset($form_state['values']['required_product_type']) ? $form_state['values']['required_product_type'] : UC_DISCOUNTS_FILTER_TYPE_NONE;
    $name = 'required_product_type';
  }
  else {
    $filter_type = isset($form_state['values']['filter_type']) ? $form_state['values']['filter_type'] : UC_DISCOUNTS_FILTER_TYPE_PRODUCTS;
    $name = 'filter_type';
  }
  $filter_name = $name;

  $options[UC_DISCOUNTS_FILTER_TYPE_PRODUCTS] = t('Filter By Products');
  $options[UC_DISCOUNTS_FILTER_TYPE_TERMS] = t('Filter By Terms');
  $options[UC_DISCOUNTS_FILTER_TYPE_SKUS] = t('Filter By SKUs');
  $options[UC_DISCOUNTS_FILTER_TYPE_CLASS] = t('Filter by Class');
  $options[UC_DISCOUNTS_FILTER_TYPE_AUTHORS] = t('Filter by Authors');
  $form[$fieldset_name]['filter'][$name] = array(
    '#type' => 'select',
    '#title' => t('Filter Type'),
    '#description' => t('Determine whether to filter by products, terms, SKUs, class, or node authors. Discount is applied to only these products.'),
    '#options' => $options,
    '#default_value' => $filter_type,
    '#required' => TRUE,
    '#attributes' => array('class' => array('filter-type')),
  );

  $name = $grouping . '-filter-select-' . UC_DISCOUNTS_FILTER_TYPE_PRODUCTS;
  $product_ids = isset($form_state['values'][$name]) ? $form_state['values'][$name] : uc_discounts_get_product_ids_for_discount_id($discount_id, $grouping, FALSE);
  $options = array();
  $result = db_query('SELECT n.nid as nid, n.title as title, p.model as model
                     FROM {uc_products} p
                     INNER JOIN {node} n ON p.nid=n.nid
                     ORDER BY title');
  if ($grouping == UC_DISCOUNTS_GROUPING_APPLICATION) {
    $options[UC_DISCOUNTS_OPTION_ALL_PRODUCTS] = t('<All Products>');
  }
  foreach ($result as $row) {
    $options[$row->nid] = $row->title . ' (' . $row->model . ')';
  }
  $form[$fieldset_name]['filter'][$name] = array(
    '#type' => 'select',
    '#title' => t('Products'),
    '#description' => t('Select all products this discount applies to or &lt;All Products&gt; to apply to all products.'),
    '#options' => $options,
    '#default_value' => $product_ids,
    '#multiple' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="' . $filter_name . '"]' => array('value' => UC_DISCOUNTS_FILTER_TYPE_PRODUCTS),
      ),
      'required' => array(
        ':input[name="' . $filter_name . '"]' => array('value' => UC_DISCOUNTS_FILTER_TYPE_PRODUCTS),
      ),
    ),
  );

  $name = $grouping . '-filter-select-' . UC_DISCOUNTS_FILTER_TYPE_TERMS;
  $term_ids = isset($form_state['values'][$name]) ? $form_state['values'][$name] : uc_discounts_get_term_ids_for_discount_id($discount_id, $grouping, FALSE);
  $options = array();
  $result = db_query('SELECT tid, name FROM {taxonomy_term_data} ORDER BY weight');
  $options[UC_DISCOUNTS_OPTION_ALL_TERMS] = t('<All Terms>');
  foreach ($result as $row) {
    $options[$row->tid] = $row->name;
  }

  $form[$fieldset_name]['filter'][$name] = array(
    '#type' => 'select',
    '#title' => t('Terms'),
    '#description' => t('Select all terms this discount applies to or &lt;All Terms&gt; to apply to all terms.  Selections are <b>not</b> recursive.'),
    '#options' => $options,
    '#default_value' => $term_ids,
    '#multiple' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="' . $filter_name . '"]' => array('value' => UC_DISCOUNTS_FILTER_TYPE_TERMS),
      ),
      'required' => array(
        ':input[name="' . $filter_name . '"]' => array('value' => UC_DISCOUNTS_FILTER_TYPE_TERMS),
      ),
    ),
  );

  $name = $grouping . '-filter-select-' . UC_DISCOUNTS_FILTER_TYPE_SKUS;
  $skus = isset($form_state['values'][$name]) ? $form_state['values'][$name] : uc_discounts_get_skus_for_discount_id($discount_id, $grouping, FALSE);
  $options = array();
  $result = db_query('SELECT p.model, n.title
                     FROM {uc_products} p, {node} n
                     WHERE p.nid = n.nid
                     ORDER BY p.model');
  if ($grouping == UC_DISCOUNTS_GROUPING_APPLICATION) {
    $options[UC_DISCOUNTS_OPTION_ALL_SKUS] = t('<All SKUs>');
  }
  foreach ($result as $row) {
    $options[$row->model] = $row->model . ' (' . $row->title . ')';
  }
  $form[$fieldset_name]['filter'][$name] = array(
    '#type' => 'select',
    '#title' => t('SKUs'),
    '#description' => t('Select all SKUs this discount applies to or &lt;All SKUs&gt; to apply to all SKUs.  Selections are <b>not</b> recursive.'),
    '#options' => $options,
    '#default_value' => $skus,
    '#multiple' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="' . $filter_name . '"]' => array('value' => UC_DISCOUNTS_FILTER_TYPE_SKUS),
      ),
      'required' => array(
        ':input[name="' . $filter_name . '"]' => array('value' => UC_DISCOUNTS_FILTER_TYPE_SKUS),
      ),
    ),
  );

  $name = $grouping . '-filter-select-' . UC_DISCOUNTS_FILTER_TYPE_CLASS;
  $classes = isset($form_state['values'][$name]) ? $form_state['values'][$name] : uc_discounts_get_classes_for_discount_id($discount_id, $grouping, FALSE);
  $options = array();
  $result = uc_product_types();
  if ($grouping == UC_DISCOUNTS_GROUPING_APPLICATION) {
    $options[UC_DISCOUNTS_OPTION_ALL_CLASSES] = t('<All Product Classes>');
  }
  foreach ($result as $type) {
    $options[$type] = $type;
  }
  $form[$fieldset_name]['filter'][$name] = array(
    '#type' => 'select',
    '#title' => t('Classes'),
    '#description' => t('Select all product classes that this discount applies to or &lt;All Classes&gt; to apply to all classes.  Selections are <b>not</b> recursive.'),
    '#options' => $options,
    '#default_value' => $classes,
    '#multiple' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="' . $filter_name . '"]' => array('value' => UC_DISCOUNTS_FILTER_TYPE_CLASS),
      ),
      'required' => array(
        ':input[name="' . $filter_name . '"]' => array('value' => UC_DISCOUNTS_FILTER_TYPE_CLASS),
      ),
    ),
  );

  $name = $grouping . '-filter-select-' . UC_DISCOUNTS_FILTER_TYPE_AUTHORS;
  $author_ids = isset($form_state['values'][$name]) ? $form_state['values'][$name] : uc_discounts_get_author_ids_for_discount_id($discount_id, $grouping, FALSE);
  $options = array();
  // TODO Please convert this statement to the D7 database API syntax.
  $result = db_query('SELECT uid, name
                     FROM {users} u
                     INNER JOIN {node} n USING(uid)
                     WHERE u.uid > 0
                     AND n.type IN (:uc_product_types)
                     GROUP BY u.uid
                     ORDER BY name', array(':uc_product_types' => uc_product_types()));
  if ($grouping == UC_DISCOUNTS_GROUPING_APPLICATION) {
    $options[UC_DISCOUNTS_OPTION_ALL_AUTHORS] = t('<All Authors>');
  }
  foreach ($result as $row) {
    $options[$row->uid] = $row->name;
  }
  $form[$fieldset_name]['filter'][$name] = array(
    '#type' => 'select',
    '#title' => t('Authors'),
    '#description' => t('Select all authors this discount applies to or &lt;All Authors&gt; to apply to all authors.'),
    '#options' => $options,
    '#default_value' => $author_ids,
    '#multiple' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="' . $filter_name . '"]' => array('value' => UC_DISCOUNTS_FILTER_TYPE_AUTHORS),
      ),
      'required' => array(
        ':input[name="' . $filter_name . '"]' => array('value' => UC_DISCOUNTS_FILTER_TYPE_AUTHORS),
      ),
    ),
  );
}

/**
 * Validate handler for uc_discounts_admin_discount_edit_form().
 *
 * @ingroup forms
 * @see uc_discounts_admin_discount_edit_form()
 * @see uc_discounts_admin_discount_edit_form_submit()
 */
function uc_discounts_admin_discount_edit_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $has_errors = FALSE;

  if ($values['requires_code'] && empty($values['codes'])) {
    form_set_error('codes', t('Discounts codes are required because %requires_code is checked', array('%requires_code' => t('Require(s) code'))));
    $has_errors = TRUE;
  }

  // Check qualifying_amount_max if necessary.
  if ($values['has_qualifying_amount_max']) {
    if (($index = strpos($values['qualifying_amount_max'], '%')) !== FALSE) {
      $value = substr($values['qualifying_amount_max'], 0, $index);
    }
    else {
      $value = $values['qualifying_amount_max'];
    }

    if (!is_numeric($value)) {
      form_set_error('qualifying_amount_max', t('Max qualifying amount is required because %has_max_qual_amount is checked.', array('%has_max_qual_amount' => t('Has max qualifying amount'))));
      $has_errors = TRUE;
    }
  }

  // Check discount_amount.
  if (($index = strpos($values['discount_amount'], '%')) !== FALSE) {
    $value = substr($values['discount_amount'], 0, $index);
  }
  else {
    $value = $values['discount_amount'];
  }

  if (!is_numeric($value)) {
    form_set_error('discount_amount', t('Discount amount must be integer, decimal or percentage.'));
    $has_errors = TRUE;
  }

  $ids = $form_state['values'][UC_DISCOUNTS_GROUPING_APPLICATION . '-filter-select-' . $values['filter_type']];
  if (empty($ids)) {
    form_set_error('filter_type', t('At least one product must be selected.'));
    $has_errors = TRUE;
  }

  if ($values['has_role_filter'] && empty($values['role_ids'])) {
    form_set_error('roles', t('Roles are required because %filter_on_roles is checked', array('%filter_on_roles' => t('Filter based on roles'))));
    $has_errors = TRUE;
  }
  /*
    if ($values['has_expiration'] && !date_is_valid($values['expiration'])) {
    form_set_error('expiration', t('Expiration required because %discount_expires is checked', array('%discount_expires' => t('Discount expires'))));
    $has_errors = TRUE;
    }
   */
  // Form requires rebuilding so codes and expiration blocks display correctly.
  if ($has_errors) {
    $form_state['rebuild'] = TRUE;
  }
}

/**
 * Submit handler for uc_discounts_admin_discount_edit_form().
 *
 * @ingroup forms
 * @see uc_discounts_admin_discount_edit_form()
 * @see uc_discounts_admin_discount_edit_form_validate()
 */
function uc_discounts_admin_discount_edit_form_submit($form, &$form_state) {
  $codes = explode("\n", $form_state['values']['codes']);

  // Set expiration to a valid value because it is not nullable.
  $expiration = isset($form_state['values']['expiration']) ? strtotime($form_state['values']['expiration']) : 0; //date_convert(date_make_date($form_state['values']['expiration']), DATE_OBJECT, DATE_UNIX) : 0;
  $activates_on = isset($form_state['values']['activates_on']) ? strtotime($form_state['values']['activates_on']) : 0; //date_convert(date_make_date($form_state['values']['activates_on']), DATE_OBJECT, DATE_UNIX) : 0;
  // Set discount_amount.
  if (($index = strpos($form_state['values']['discount_amount'], '%')) !== FALSE) {
    $discount_amount = floatval(substr($form_state['values']['discount_amount'], 0, $index)) / 100;
  }
  else {
    $discount_amount = floatval($form_state['values']['discount_amount']);
  }

  $discount = new stdClass();
  $discount->name = $form_state['values']['name'];
  $discount->short_description = $form_state['values']['short_description'];
  $discount->description = $form_state['values']['description'];
  $discount->qualifying_type = $form_state['values']['qualifying_type'];
  $discount->qualifying_amount = $form_state['values']['qualifying_amount'];
  $discount->has_qualifying_amount_max = $form_state['values']['has_qualifying_amount_max'];
  $discount->qualifying_amount_max = $form_state['values']['qualifying_amount_max'];
  $discount->discount_type = $form_state['values']['discount_type'];
  $discount->discount_amount = $discount_amount;
  $discount->requires_code = $form_state['values']['requires_code'];
  $discount->filter_type = $form_state['values']['filter_type'];
  $discount->has_role_filter = $form_state['values']['has_role_filter'];
  $discount->use_only_discounted_products_to_qualify = $form_state['values']['use_only_discounted_products_to_qualify'];
  $discount->requires_single_product_to_qualify = $form_state['values']['requires_single_product_to_qualify'];
  $discount->required_product_type = $form_state['values']['required_product_type'];
  $discount->add_to_cart = $form_state['values']['add_to_cart'];
  $discount->max_times_applied = $form_state['values']['max_times_applied'];
  $discount->limit_max_times_applied = $form_state['values']['limit_max_times_applied'];
  $discount->can_be_combined_with_other_discounts = $form_state['values']['can_be_combined_with_other_discounts'];
  $discount->max_uses = $form_state['values']['max_uses'];
  $discount->max_uses_per_user = $form_state['values']['max_uses_per_user'];
  $discount->max_uses_per_code = $form_state['values']['max_uses_per_code'];
  $discount->has_expiration = $form_state['values']['has_expiration'];
  $discount->expiration = $expiration;
  $discount->has_activation = $form_state['values']['has_activation'];
  $discount->activates_on = $activates_on;
  $discount->is_active = $form_state['values']['is_active'];
  $discount->weight = $form_state['values']['weight'];
  if (empty($form_state['values']['discount_id'])) {
    $discount->insert_timestamp = REQUEST_TIME;
    drupal_write_record('uc_discounts', $discount);
  }
  else {
    $discount->discount_id = $form_state['values']['discount_id'];
    drupal_write_record('uc_discounts', $discount, 'discount_id');
    // Delete existing related fields.
    uc_discounts_codes_delete($discount->discount_id);
    uc_discounts_products_delete($discount->discount_id);
    uc_discounts_terms_delete($discount->discount_id);
    uc_discounts_skus_delete($discount->discount_id);
    uc_discounts_classes_delete($discount->discount_id);
    uc_discounts_authors_delete($discount->discount_id);
    uc_discounts_roles_delete($discount->discount_id);
  }

  // Insert codes if necessary.
  if ($form_state['values']['requires_code']) {
    foreach ($codes as $code) {
      $code = trim($code);
      if (empty($code)) {
        continue;
      }
      $new_code = array(
        'discount_id' => $discount->discount_id,
        'code' => $code,
      );
      drupal_write_record('uc_discounts_codes', $new_code);
    }
  }

  // Insert required products.
  foreach (array(UC_DISCOUNTS_GROUPING_APPLICATION, UC_DISCOUNTS_GROUPING_QUALIFICATION) as $grouping) {
    $name = $grouping == UC_DISCOUNTS_GROUPING_APPLICATION ? 'filter_type' : 'required_product_type';
    if ($form_state['values'][$name] == UC_DISCOUNTS_FILTER_TYPE_PRODUCTS) {
      $ids = $form_state['values'][$grouping . '-filter-select-' . UC_DISCOUNTS_FILTER_TYPE_PRODUCTS];
      $ids = in_array(UC_DISCOUNTS_OPTION_ALL_PRODUCTS, $ids) ? array(UC_DISCOUNTS_OPTION_ALL_PRODUCTS) : array_filter($ids);
      foreach ($ids as $id) {
        $new_product = array(
          'discount_id' => $discount->discount_id,
          'product_id' => $id,
          'grouping' => $grouping,
        );
        drupal_write_record('uc_discounts_products', $new_product);
      }
    }
    elseif ($form_state['values'][$name] == UC_DISCOUNTS_FILTER_TYPE_TERMS) {
      $ids = $form_state['values'][$grouping . '-filter-select-' . UC_DISCOUNTS_FILTER_TYPE_TERMS];
      $ids = in_array(UC_DISCOUNTS_OPTION_ALL_TERMS, $ids) ? array(UC_DISCOUNTS_OPTION_ALL_TERMS) : array_filter($ids);
      foreach ($ids as $id) {
        $new_term = array(
          'discount_id' => $discount->discount_id,
          'term_id' => $id,
          'grouping' => $grouping,
        );
        drupal_write_record('uc_discounts_terms', $new_term);
      }
    }
    elseif ($form_state['values'][$name] == UC_DISCOUNTS_FILTER_TYPE_SKUS) {
      $ids = $form_state['values'][$grouping . '-filter-select-' . UC_DISCOUNTS_FILTER_TYPE_SKUS];
      // Empty SKU is UC_DISCOUNTS_OPTION_ALL_SKUS.
      $ids = in_array(UC_DISCOUNTS_OPTION_ALL_SKUS, $ids) ? array() : array_filter($ids);
      foreach ($ids as $id) {
        $new_sku = array(
          'discount_id' => $discount->discount_id,
          'sku' => $id,
          'grouping' => $grouping,
        );
        drupal_write_record('uc_discounts_skus', $new_sku);
      }
    }
    elseif ($form_state['values'][$name] == UC_DISCOUNTS_FILTER_TYPE_CLASS) {
      $ids = $form_state['values'][$grouping . '-filter-select-' . UC_DISCOUNTS_FILTER_TYPE_CLASS];
      // Empty Class is UC_DISCOUNTS_OPTION_ALL_CLASSES.
      $ids = in_array(UC_DISCOUNTS_OPTION_ALL_CLASSES, $ids) ? array() : array_filter($ids);
      foreach ($ids as $id) {
        $new_class = array(
          'discount_id' => $discount->discount_id,
          'class' => $id,
          'grouping' => $grouping,
        );
        drupal_write_record('uc_discounts_classes', $new_class);
      }
    }
    if ($form_state['values'][$name] == UC_DISCOUNTS_FILTER_TYPE_AUTHORS) {
      $ids = $form_state['values'][$grouping . '-filter-select-' . UC_DISCOUNTS_FILTER_TYPE_AUTHORS];
      $ids = in_array(UC_DISCOUNTS_OPTION_ALL_AUTHORS, $ids) ? array(UC_DISCOUNTS_OPTION_ALL_AUTHORS) : array_filter($ids);
      foreach ($ids as $id) {
        $new_author = array(
          'discount_id' => $discount->discount_id,
          'author_id' => $id,
          'grouping' => $grouping,
        );
        drupal_write_record('uc_discounts_authors', $new_author);
      }
    }
  }

  // Insert roles.
  if ($form_state['values']['has_role_filter']) {
    $role_ids = ($form_state['values']['has_role_filter']) ? $form_state['values']['role_ids'] : array();
    $role_ids = in_array(UC_DISCOUNTS_OPTION_ALL_ROLES, $role_ids) ? array(UC_DISCOUNTS_OPTION_ALL_ROLES) : array_filter($role_ids);
    foreach ($role_ids as $role_id) {
      $new_role = array(
        'discount_id' => $discount->discount_id,
        'role_id' => $role_id,
      );
      drupal_write_record('uc_discounts_roles', $new_role);
    }
  }

  foreach (module_implements('uc_discount') as $module) {
    $function = $module . '_uc_discount';
    $function('save', $discount);
  }

  drupal_set_message(t('%name settings have been saved.', array('%name' => $form_state['values']['name'])));
  $form_state['redirect'] = 'admin/store/uc_discounts/edit/' . $discount->discount_id;
}

/**
 * Generates admin form to create a copy of existing discount.
 *
 * @ingroup forms
 */
function uc_discounts_admin_discount_copy_form($original_discount_id) {
  $discount = uc_discounts_load($original_discount_id);
  $discount->name = t('Copy of @discount', array('@discount' => $discount->name));
  $discount->insert_timestamp = REQUEST_TIME;
  unset($discount->discount_id);
  drupal_write_record('uc_discounts', $discount);

  // Copy discount's codes.
  $codes = uc_discounts_get_codes_for_discount_id($original_discount_id);
  foreach ($codes as $code) {
    $new_code = array(
      'discount_id' => $discount->discount_id,
      'code' => $code,
    );
    drupal_write_record('uc_discounts_codes', $new_code);
  }

  foreach (array(UC_DISCOUNTS_GROUPING_APPLICATION, UC_DISCOUNTS_GROUPING_QUALIFICATION) as $grouping) {
    // Copy discount's products.
    $product_ids = uc_discounts_get_product_ids_for_discount_id($original_discount_id, $grouping);
    foreach ($product_ids as $product_id) {
      $new_product = array(
        'discount_id' => $discount->discount_id,
        'product_id' => $product_id,
        'grouping' => $grouping,
      );
      drupal_write_record('uc_discounts_products', $new_product);
    }

    // Copy discount's terms.
    $term_ids = uc_discounts_get_term_ids_for_discount_id($original_discount_id, $grouping);
    foreach ($term_ids as $term_id) {
      $new_term = array(
        'discount_id' => $discount->discount_id,
        'term_id' => $term_id,
        'grouping' => $grouping,
      );
      drupal_write_record('uc_discounts_terms', $new_term);
    }

    // Copy discount's SKUs.
    $skus = uc_discounts_get_skus_for_discount_id($original_discount_id, $grouping);
    foreach ($skus as $sku) {
      $new_sku = array(
        'discount_id' => $discount->discount_id,
        'sku' => $sku,
        'grouping' => $grouping,
      );
      drupal_write_record('uc_discounts_skus', $new_sku);
    }

    // Copy discount's Classes.
    $classes = uc_discounts_get_classes_for_discount_id($original_discount_id, $grouping);
    foreach ($classes as $class) {
      $new_class = array(
        'discount_id' => $discount->discount_id,
        'class' => $class,
        'grouping' => $grouping,
      );
      drupal_write_record('uc_discounts_classes', $new_class);
    }

    // Copy discount's authors.
    $author_ids = uc_discounts_get_author_ids_for_discount_id($original_discount_id, $grouping);
    foreach ($author_ids as $author_id) {
      $new_author = array(
        'discount_id' => $discount->discount_id,
        'author_id' => $author_id,
        'grouping' => $grouping,
      );
      drupal_write_record('uc_discounts_authors', $new_author);
    }
  }

  // Copy discount's roles.
  $role_ids = uc_discounts_get_role_ids_for_discount_id($original_discount_id);
  foreach ($role_ids as $role_id) {
    $new_role = array(
      'discount_id' => $discount->discount_id,
      'role_id' => $role_id,
    );
    drupal_write_record('uc_discounts_roles', $new_role);
  }

  foreach (module_implements('uc_discount') as $module) {
    $function = $module . '_uc_discount';
    $function('save', $discount);
  }
  drupal_set_message(t('The discount was successfully copied as %name.', array('%name' => $discount->name)));
  drupal_goto('admin/store/uc_discounts/edit/' . $discount->discount_id);
}

/**
 * Generates an admin form to delete a discount.
 *
 * @ingroup forms
 * @see uc_discounts_admin_discount_delete_form_submit()
 */
function uc_discounts_admin_discount_delete_form($form, &$form_state, $discount_id) {
  $discount = uc_discounts_load($discount_id);
  $form = array(
    'discount_id' => array(
      '#type' => 'value',
      '#value' => $discount_id,
    ),
  );
  $output = confirm_form($form, t('Delete @row?', array('@row' => $discount->name)), 'admin/store/uc_discounts');
  return $output;
}

/**
 * Submit handler for uc_discounts_admin_discount_delete_form().
 *
 * @ingroup forms
 * @see uc_discounts_admin_discount_delete_form()
 */
function uc_discounts_admin_discount_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $discount = uc_discounts_load($form_state['values']['discount_id']);
    uc_discounts_delete_all($discount);
    drupal_set_message(t('Discount deleted.'));
  }

  $form_state['redirect'] = 'admin/store/uc_discounts';
}

/**
 * Generate admin report page content of all discount usage.
 *
 * @return array
 *   Array of page content.
 */
function uc_discounts_admin_report_discounts_list($download = FALSE) {
  $op = arg(4) == 'download' ? 'download' : 'show';
  $header = array(
    array(
      'data' => t('Active'),
      'field' => 'd.is_active',
    ),
    array(
      'data' => t('Name'),
      'field' => 'd.name',
    ),
    array(
      'data' => t('Short Description'),
      'field' => 'd.short_description',
    ),
    array(
      'data' => t('Qualifying Type'),
      'field' => 'd.qualifying_type',
    ),
    array(
      'data' => t('Type'),
      'field' => 'd.discount_type',
    ),
    array(
      'data' => t('Uses'),
      'field' => 'use_count',
    ),
    array(
      'data' => t('Times Applied'),
      'field' => 'total_times_applied',
    ),
    array(
      'data' => t('Discounted Amount'),
      'field' => 'total_amount',
    ),
    array(
      'data' => t('Revenue Amount'),
      'field' => 'total_revenue',
    ),
    array(
      'data' => t('Weight'),
      'field' => 'd.weight',
    ),
    array(
      'data' => t('Created At'),
      'field' => 'insert_timestamp',
      'sort' => 'desc',
    ),
    array('data' => t('Operations')),
  );

  // TODO Please convert this statement to the D7 database API syntax.
  $select = db_select('uc_discounts', 'd')
    ->extend('TableSort')
    ->orderByHeader($header);
  if ($op != 'download') {
    $select = $select
      ->extend('PagerDefault')
      ->limit(50);
  }
  $select
    ->fields('d')
    ->groupBy('d.discount_id');

  $select->leftjoin('uc_discounts_uses', 'du', 'd.discount_id=du.discount_id');
  $select->leftjoin('uc_orders', 'o', 'o.order_id=du.order_id AND o.order_status = :order_status AND o.order_total > 0', array(':order_status' => 'completed'));

  $select->addExpression('COUNT(du.discount_use_id)', 'total_use_count');
  $select->addExpression('SUM(du.times_applied)', 'total_times_applied');
  $select->addExpression('SUM(du.amount)', 'total_amount');
  $select->addExpression('SUM(o.order_total)', 'total_revenue');

  $result = $select->execute();

  $rows = array();
  foreach ($result as $discount) {
    $total_use_count = (is_numeric($discount->total_use_count)) ? $discount->total_use_count : 0;
    $total_times_applied = (is_numeric($discount->total_times_applied)) ? $discount->total_times_applied : 0;
    $operations = array(
      l(t('usage'), 'admin/reports/uc_discounts/discount/' . $discount->discount_id),
      l(t('edit'), 'admin/store/uc_discounts/edit/' . $discount->discount_id),
      l(t('delete'), 'admin/store/uc_discounts/delete/' . $discount->discount_id),
    );

    $rows[] = array(
      array(
        'data' => ($discount->is_active ? '&#10003;' : 'X'),
        'class' => array(($discount->is_active ? 'is_active' : 'is_inactive')),
      ),
      $discount->name,
      $discount->short_description,
      uc_discounts_qualifying_type_name($discount->qualifying_type),
      uc_discounts_discount_type_name($discount->discount_type),
      array(
        'data' => $total_use_count,
        'class' => array('numeric'),
      ),
      array(
        'data' => $total_times_applied,
        'class' => array('numeric'),
      ),
      uc_currency_format($discount->total_amount),
      uc_currency_format($discount->total_revenue),
      array(
        'data' => $discount->weight,
        'class' => array('weight'),
      ),
      array(
        'data' => format_date($discount->insert_timestamp, 'short'),
        'class' => array('date'),
      ),
      array(
        'data' => implode(' ', $operations),
        'class' => array('operations'),
      ),
    );
  }

  if ($op == 'download') {
    $http_headers = array(
      'Pragma: no-cache',
      'Expires: 0',
      'Cache-Control: no-cache, must-revalidate',
      'Cache-Control: private',
      'Content-Transfer-Encoding: binary',
      'Content-Disposition: attachment; filename="discount_usage_report.csv"',
      'Content-Type: text/csv',
    );
    foreach ($http_headers as $http_header) {
      $http_header = preg_replace('/\r?\n(?!\t| )/', '', $http_header);
      drupal_add_http_header($http_header);
    }

    $fp = fopen('php://output', 'w');
    array_unshift($rows, $header);
    foreach ($rows as $key => $row) {
      $csv_row = array();
      // Remove Operations cell.
      array_pop($row);
      foreach ($row as $cell) {
        $csv_row[] = is_array($cell) ? $cell['data'] : $cell;
      }
      if ($key > 0) {
        // Make is_active CSV friendly.
        $csv_row[0] = $csv_row[0] == 'X' ? 0 : 1;
      }
      fputcsv($fp, $csv_row);
    }
    fclose($fp);
    exit;
  }
  else {
    if (empty($rows)) {
      $rows[] = array(
        array(
          'data' => t('No discounts.'),
          'colspan' => count($header),
        ),
      );
    }

    $output['content'] = array(
      'table' => array(
        '#markup' => theme('table', array(
          'header' => $header,
          'rows' => $rows,
          'attributes' => array(
            'class' => array('uc-discounts'),
          ),
        )),
      ),
      'pager' => array('#theme' => 'pager', '#weight' => 5),
      'link' => array('#markup' => l(t('Export to CSV'), 'admin/reports/uc_discounts/all/download')),
    );

    return $output;
  }
}

/**
 * Generate admin report page content of discount usage for discount ID.
 *
 * @param int $discount_id
 *
 * @return array
 *   Array of page content.
 */
function uc_discounts_admin_report_discount($discount_id) {
  $output = array();
  // Add link to all discount data.
  $output['content']['title'] = array('#markup' => l(t('Report for all discounts.'), 'admin/reports/uc_discounts/all') . '<br /><br />');

  // Add table for discount's data.

  $header = array(
    t('Name'),
    t('Uses'),
    t('Times Applied'),
    t('Discounted Amount'),
    t('Revenue Amount'),
    array(
      'data' => t('Operations'),
      'colspan' => 3,
    ),
  );

  $discount = db_query('SELECT d.*, COUNT(du.discount_use_id) total_use_count,
                            SUM(du.times_applied) total_times_applied, SUM(du.amount) total_amount, SUM(o.order_total) total_revenue
                          FROM {uc_discounts} d
                          LEFT JOIN {uc_discounts_uses} du ON d.discount_id=du.discount_id
                          LEFT JOIN {uc_orders} o ON du.order_id=o.order_id AND o.order_status = "completed" AND o.order_total > 0
                          WHERE d.discount_id = :d_discount_id
                          GROUP BY d.discount_id', array(':d_discount_id' => $discount_id))->fetchObject();
  $total_use_count = (is_numeric($discount->total_use_count)) ? $discount->total_use_count : 0;
  $total_times_applied = (is_numeric($discount->total_times_applied)) ? $discount->total_times_applied : 0;
  $discounts[] = array(
    $discount->name,
    $total_use_count,
    $total_times_applied,
    uc_currency_format($discount->total_amount),
    uc_currency_format($discount->total_revenue),
    l(t('edit'), 'admin/store/uc_discounts/edit/' . $discount->discount_id),
    l(t('copy'), 'admin/store/uc_discounts/copy/' . $discount->discount_id),
    l(t('delete'), 'admin/store/uc_discounts/delete/' . $discount->discount_id),
  );
  $output['content']['table1'] = array(
    '#markup' => theme('table', array(
      'header' => $header,
      'rows' => $discounts,
    )) . '<br /><br />',
  );

  // Add table of discount's usage data.
  $header = array(
    array(
      'data' => t('User'),
      'field' => 'user_id',
    ),
    array(
      'data' => t('Order'),
      'field' => 'order_id',
    ),
    array(
      'data' => t('Code'),
      'field' => 'code',
    ),
    array(
      'data' => t('Times Applied'),
      'field' => 'times_applied',
    ),
    array(
      'data' => t('Amount'),
      'field' => 'amount',
    ),
    array(
      'data' => t('Date'),
      'field' => 'insert_timestamp',
      'sort' => 'asc',
    ),
  );

  $select = db_select('uc_discounts_uses', 'du')
    ->extend('PagerDefault')
    ->limit(50)
    ->extend('TableSort')
    ->orderByHeader($header)
    ->fields('du')
    ->condition('discount_id', $discount_id);

  $select->leftjoin('users', 'u', 'du.user_id=u.uid');
  $select->addExpression('u.uid', 'user_id');
  $select->addExpression('u.name', 'username');
  $select->addExpression('u.mail', 'email');

  $result = $select->execute();

  $rows = array();
  foreach ($result as $use) {
    $user_description = ($use->user_id != 0) ? $use->username . ' (' . $use->email . ')' : t('Anonymous');

    $rows[] = array(
      'data' => array(
        $user_description,
        $use->order_id,
        (!empty($use->code)) ? $use->code : '<' . t('no code') . '>',
        $use->times_applied,
        uc_currency_format($use->amount),
        date('Y-m-d H:i:s', $use->insert_timestamp),
      ),
    );
  }

  if (empty($rows)) {
    $rows[] = array(
      'data' => array(
        'data' => t('No discount data available.'),
        'colspan' => 11,
      ),
    );
  }

  $output['content']['table2'] = array(
    '#markup' => theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array('id' => 'uc_discounts_report'),
    )),
  );
  $output['content']['pager'] = array('#theme' => 'pager', '#weight' => 5);

  return $output;
}

/**
 * Generate unique discount codes for a specified discount id.
 */
function uc_discounts_admin_generate_codes(&$form_state, $discount_id) {

}

/**
 * Generate an admin form used to automatically create discount codes.
 *
 * @param array $form
 * @param array $form_state
 * @param int $discount_id
 * 
 * @return string
 *
 * @ingroup forms
 * @see uc_discounts_admin_generate_codes()
 * @see uc_discounts_admin_generate_codes_form_submit()
 * @see uc_discounts_admin_generate_codes_form_validate()
 */
function uc_discounts_admin_generate_codes_form($form, &$form_state, $discount_id) {
  $form['about'] = array(
    '#markup' => '<p>' . t('Generate unqiue codes for this discount.  After generating them simply copy and paste them into the Codes textarea for your discount.') . '</p>',
  );
  $form['discount_link'] = array(
    '#markup' => '<p>' . t('Discount') . '#: ' . l($discount_id, 'admin/store/uc_discounts/edit/' . $discount_id) . '</p>',
  );
  $form['num'] = array(
    '#title' => t('Number of codes'),
    '#type' => 'textfield',
    '#default_value' => isset($form_state['values']['num']) ? $form_state['values']['num'] : '',
    '#required' => TRUE,
    '#size' => 5,
  );
  $form['pattern'] = array(
    '#title' => t('Code pattern'),
    '#description' => t('The pattern for generating the codes.  %d will be replaced with a random digit, %s will be replaced with a random letter.'),
    '#default_value' => isset($form_state['values']['pattern']) ? $form_state['values']['pattern'] : 'code%d%s%d',
    '#type' => 'textfield',
    '#required' => TRUE,
    '#size' => 15,
  );
  $form['discount_id'] = array(
    '#type' => 'value',
    '#value' => $discount_id,
  );
  $form['submit'] = array(
    '#value' => t('Generate codes'),
    '#type' => 'submit',
  );

  if (!empty($_SESSION['codes'])) {
    $form['codes'] = array(
      '#markup' => '<p><em>' . t('Copy these codes into the Codes textarea for your discount. Note that this list also contains any existing codes for the discount.')
      . '</em></p><code class="uc-discounts-generated-codes">' . $_SESSION['codes'] . '</code>',
    );
    unset($_SESSION['codes']);
  }

  return $form;
}

/**
 * Form validation handler for uc_discounts_admin_generate_codes_form().
 *
 * @ingroup forms
 * @see uc_discounts_admin_generate_codes()
 * @see uc_discounts_admin_generate_codes_form()
 * @see uc_discounts_admin_generate_codes_form_submit()
 */
function uc_discounts_admin_generate_codes_form_validate($form, &$form_state) {
  if (!is_numeric($form_state['values']['num']) || $form_state['values']['num'] <= 0) {
    form_set_error('num', t('Number of discounts must be a number greater than 0.'));
  }

  if (substr_count($form_state['values']['pattern'], '%') < 2) {
    form_set_error('pattern', t('Code pattern must have at least two random digits or letters to ensure uniqueness.'));
  }
}

/**
 * Submit handler for uc_discounts_generate_codes_form().
 *
 * @ingroup forms
 * @see uc_discounts_admin_generate_codes()
 * @see uc_discounts_admin_generate_codes_form()
 * @see uc_discounts_admin_generate_codes_form_validate()
 */
function uc_discounts_admin_generate_codes_form_submit($form, &$form_state) {
  $codes = uc_discounts_get_codes_for_discount_id($form_state['values']['discount_id']);
  foreach (range(1, $form_state['values']['num']) as $num) {
    while (TRUE) {
      $code = preg_replace_callback('/%(s|d)/', '_uc_discounts_get_random_string', $form_state['values']['pattern']);
      if (!in_array($code, $codes)) {
        $codes[] = $code;
        break;
      }
    }
  }

  $_SESSION['codes'] = implode("\n", $codes);
}

/**
 * Generates a random digit or letter, used as a callback for preg_replace.
 */
function _uc_discounts_get_random_string($matches) {
  // @see user_password
  $str = $matches[1] == 's' ? 'abcdefghijkmnopqrstuvwxyz' : '23456789';
  $str_len = strlen($str) - 1;
  return $str[mt_rand(0, $str_len)];
}

/**
 * Displays admin settings form for discounts checkout pane.
 */
function uc_discounts_admin_checkout_pane_settings_form($form, &$form_state) {
  $form['uc_discounts_checkout_allow_user_codes'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow customer to enter discount codes'),
    '#description' => t('When enabled a text field will be displayed to the'
      . ' customer during checkout where they can enter discount codes.'),
    '#default_value' => variable_get('uc_discounts_checkout_allow_user_codes', TRUE),
  );
  $form['uc_discounts_checkout_show_messages'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show messages about discounts in discounts checkout pane'),
    '#description' => t('When enabled messages about call discounts will be'
      . ' displayed to the customer during checkout in the discounts checkout'
      . ' pane. This provides additional information beyond the discount text'
      . ' displayed in the checkout cart pane and checkout payment pane.'),
    '#default_value' => variable_get('uc_discounts_checkout_show_messages', TRUE),
  );
  return $form;
}
