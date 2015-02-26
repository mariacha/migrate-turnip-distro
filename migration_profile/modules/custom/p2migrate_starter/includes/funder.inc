<?php

/**
 * Class to migrate Funder content type to a new user.
 */
class DrupalFunderMigration extends DrupalMigration {
  /**
   * Construction.
   */
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'nid' => array('type' => 'int',
                       'unsigned' => TRUE,
                       'not null' => TRUE,
                       'description' => 'Source Funder nid',
                       'alias' => 'n',
                      ),
      ),
      DrupalDestinationUser::getKeySchema()
    );
    $this->source = new MigrateSourceSQL($this->query(), array(), NULL, array('map_joinable' => FALSE));

    $this->destination = new DrupalDestinationUser();

    $this->addFieldMapping('name', 'title');
    $this->addFieldMapping('role_names')
        ->defaultValue('Funder');

    $this->addFieldMapping('pass')
        ->callbacks(array($this, 'generatePassword'))
        ->defaultValue('this will not be used');

    $this->addFieldMapping('og_user_node','sponsored')
        ->separator(',')
        ->sourceMigration(array('Decks','Schools','Classrooms'));

    $this->addFieldMapping('field_user_picture', 'field_funder_logo_fid')
        ->sourceMigration('Files');

    $this->addFieldMapping('field_user_picture:file_class')
        ->defaultValue('MigrateFileFid');

    $this->addFieldMapping('field_user_picture:alt', 'title');
    $this->addFieldMapping('field_user_picture:title', 'title');

    $this->addFieldMapping('field_bio_100', 'body');

    $this->addFieldMapping('field_bio_100:format')
        ->defaultValue('filtered_html');

    $this->addSimpleMappings(array('created', 'status'));

    $this->addUnmigratedSources(array(
      'vid',
      'language',
      'uid',
      'changed',
      'comment',
      'promote',
      'moderate',
      'sticky',
      'tnid',
      'translate',
    ));

    $this->addUnmigratedDestinations(array(
      'mail',
      'access',
      'login',
      'roles',
      'picture',
      'signature',
      'signature_format',
      'timezone',
      'language',
      'theme',
      'init',
      'data',
      'is_new',
      'path',
      'pathauto',
      'field_user_picture:language',
      'field_user_picture:preserve_files',
      'field_bio_100:language',
    ));
  }

  /**
   * Generates a random password for the user.
   */
  protected function generatePassword($pass) {
    return user_password();
  }

  /**
   * Query for the basic user data. Same query is used for all currently-supported
   * versions of Drupal.
   *
   * @return QueryConditionInterface
   */
  protected function query() {
    // Do not attempt to migrate the anonymous user row.
    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('node', 'n')
             ->fields('n', array('nid', 'vid', 'language', 'title',
                 'uid', 'status', 'created', 'changed', 'comment', 'promote',
                 'moderate', 'sticky', 'tnid', 'translate'))
             ->condition('n.type', 'funder')
             ->orderBy('n.changed');

    $query->innerJoin('node_revisions', 'nr', 'n.vid=nr.vid');
    $query->fields('nr', array('body'));

    $query->innerJoin('content_type_funder', 'f', 'n.vid=f.vid');
    $query->fields('f', array('field_funder_logo_fid'));

    $query->addExpression('(SELECT group_concat(field_funder_schools_nid) FROM content_field_funder_schools cffs WHERE cffs.vid=n.vid)', 'sponsored');

    return $query;
  }
}

/**
 * Class to migrate Funder content type to a new user's Funder profile.
 */
class DrupalFunderProfileMigration extends DrupalMigration {
  /**
   * Construction.
   */
  public function __construct(array $arguments) {

    parent::__construct($arguments);

    $this->sourceFields += $this->version->getSourceFields('node', $this->sourceType);

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'nid' => array('type' => 'int',
                       'unsigned' => TRUE,
                       'not null' => TRUE,
                       'description' => 'Source Funder nid',
                      ),
      ),
      MigrateDestinationProfile2::getKeySchema()
    );
	  $this->sourceOptions['fix_field_names'] = array();
    $this->source = new MigrateDrupal6SourceSQL($this->query(), $this->sourceFields, NULL,
      $this->sourceOptions);

    $this->destination = new MigrateDestinationProfile2('funder');

    $this->addFieldMapping('uid', 'nid')
        ->sourceMigration('Funder');

    $this->addFieldMapping('field_funder_url', 'field_funder_url');

    $this->addUnmigratedSources(array(
      'vid',
      'language',
      'uid',
      'status',
      'changed',
      'created',
      'comment',
      'promote',
      'moderate',
      'sticky',
      'tnid',
      'translate',
      'teaser',
      'format',
    ));
  }

  /**
   * Query for basic node fields from Drupal 6.
   *
   * Copied from the DrupalNode6Migration class. Can't use the class by default
   * because the destination is a profile2 instead of another node.
   *
   * @return QueryConditionInterface
   */
  protected function query() {
    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('node', 'n')
             ->fields('n', array('nid', 'vid', 'language', 'title',
                 'uid', 'status', 'created', 'changed', 'comment', 'promote',
                 'moderate', 'sticky', 'tnid', 'translate'))
             ->condition('n.type', $this->sourceType)
             ->orderBy('n.changed');
    $query->innerJoin('node_revisions', 'nr', 'n.vid=nr.vid');
    $query->fields('nr', array('body', 'teaser', 'format'));
    // Pick up simple CCK fields
    $cck_table = 'content_type_' . $this->sourceType;
    if (Database::getConnection('default', $this->sourceConnection)
          ->schema()->tableExists($cck_table)) {
      $query->leftJoin($cck_table, 'f', 'n.vid=f.vid');
      // The main column for the field should be rendered with
      // the field name, not the column name (e.g., field_foo rather
      // than field_foo_value).
      $field_info = $this->version->getSourceFieldInfo();
      foreach ($field_info as $field_name => $info) {
        if (isset($info['columns']) && !$info['multiple'] && $info['db_storage']) {
          $i = 0;
          $data = FALSE;
          foreach ($info['columns'] as $display_name => $column_name) {
            if ($i++ == 0) {
              $query->addField('f', $column_name, $field_name);
            }
            else {
              // The database API won't allow colons in column aliases, so we
              // will accept the default alias, and fix up the field names later.
              // Remember how to translate the field names.
              $clean_name = str_replace(':', '_', $display_name);
              $this->fixFieldNames[$clean_name] = $display_name;
              if ($info['type'] == 'filefield' &&
                  (strpos($display_name, ':list') || strpos($display_name, ':description') || strpos($display_name, ':alt'))) {
                if (!$data) {
                  $this->fileDataFields[] = $field_name . '_data';
                  $query->addField('f', $field_name . '_data');
                  $data = TRUE;
                }
              }
              else {
                $query->addField('f', $column_name);
              }
            }
          }
        }
      }
    }
    return $query;
  }
}
