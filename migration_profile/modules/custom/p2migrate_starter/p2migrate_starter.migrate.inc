<?php
/**
 * @file
 * Contains migrate module hooks.
 */

/**
 * Implements hook_flush_caches().
 *
 * Re-find the migrations every time you flush caches.
 */
function p2migrate_starter_flush_caches() {
  p2migrate_starter_register_migrations();
}

/**
 * Implements hook_migrate_api().
 *
 * Sets up the migration(s).
 */
function p2migrate_starter_migrate_api() {
  // Set source file directory. This variable should be set in
  // `settings.local.php`.
  $file_path = variable_get('p2migrate_starter_drupal_source_file_directory', FALSE);
  if (!file_exists($file_path)) {
    throw new \Exception(sprintf('No file directory found at %s. If you plan to migrate files, please add the variable "p2migrate_starter_drupal_source_file_directory" to your settings.php file.', $file_path));
  }
  $api = array(
    'api' => 2,
    'groups' => array(
      'simple_example' => array(
        'title' => t('Example migration with from D6 to D7.'),
      ),
    ),
  );

  return $api;
}

/**
 * Register all D6->D7 migrations.
 */
function p2migrate_starter_register_migrations() {
  /**
   * Each migration being registered takes an array of arguments, some required
   * and some optional. Start with the common arguments required by all - the
   * source_connection (connection key, set up in settings.php, pointing to
   * the Drupal 6 database), source_version (major version of Drupal), and
   * group_name (a.k.a. import job).
   */

  // Begin migration: Roles.
  $common_arguments = array(
    'source_connection' => 'legacydrupal',
    'source_version' => 6,
    'group_name' => 'simple_example',
  );

  // Begin migration: Files.
  $arguments = $common_arguments + array(
    'description' => t('Migration of files from Drupal 6'),
    'machine_name' => 'Files',
    'default_uid' => 1,
    'source_dir' => variable_get('p2migrate_starter_drupal_source_file_directory', FALSE),
    'destination_dir' => 'public://legacy_files',
  );

  Migration::registerMigration('DrupalFile6Migration', $arguments['machine_name'],
                               $arguments);
}
