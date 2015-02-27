<?php
/**
 * @file
 * p2p_test.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function p2p_test_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => "1");
  }
}

/**
 * Implements hook_default_profile2_type().
 */
function p2p_test_default_profile2_type() {
  $items = array();
  $items['main'] = entity_import('profile2_type', '{
    "userCategory" : true,
    "userView" : true,
    "type" : "main",
    "label" : "Main profile",
    "weight" : "0",
    "data" : { "registration" : true, "use_page" : true },
    "rdf_mapping" : []
  }');
  $items['secondary_profile_type'] = entity_import('profile2_type', '{
    "userCategory" : true,
    "userView" : true,
    "type" : "secondary_profile_type",
    "label" : "Secondary profile type",
    "weight" : "0",
    "data" : { "registration" : 0 },
    "rdf_mapping" : []
  }');
  return $items;
}
