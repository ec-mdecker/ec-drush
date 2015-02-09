<?php

require(dirname(__FILE__) . '/../terminus/terminus.user.api.inc');


function eaglecrk_drush_command() {
  $items = array();
  $items['eaglecrk-config'] = array(
    'description' => "Configure your server settings",
    'aliases' => array('ec-config'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );
  $items['eaglecrk-clone-site'] = array(
    'description' => "Clones a site into local development",
    'aliases' => array('ec-clone'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['eaglecrk-show-config'] = array(
    'description' => "Displays current configurations.",
    'aliases' => array('ec-show-config'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['eaglecrk-restore-database'] = array(
    'description' => 'Download and overwrite an existing database',
    'aliases' => array('ec-redb'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['eaglecrk-clear-terminus'] = array(
    'description' => "Remove stored variables such as configurations and login info.",
    'aliases' => array('ec-clear'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['eaglecrk-create-site'] = array(
    'description' => 'Create a site with the Eagle Creek Distribution.',
    'aliases' => array('ec-create'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['eaglecrk-test'] = array(
    'description' => 'teststuff.',
    'aliases' => array('ectest'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['eaglecrk-test2'] = array(
    'description' => 'teststuff.',
    'aliases' => array('ectest2'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  return $items;
}

function drush_eaglecrk_test() {
  $file = fopen('/home/mdecker/Downloads/jcplogs - welcomeKiddieKandids.csv', 'r');
  $out_file = fopen('/home/mdecker/Downloads/jcplogsKiddie.csv', 'w');
  $i = 0;
  while (!feof($file)) {
    $line = fgetcsv($file);
    if (is_array($line)) {
      foreach ($line as &$item) {
        $item = trim($item);
      }

      if (strrpos($line[0], ' ') === FALSE) {
        $i++;
        $ip = $line[0];
        $country = shell_exec("wget -q -O- http://api.wipmania.com/$ip");
        drush_print($country . ' - ' . $i);
        $data = array('ip' => $ip, 'country' => $country, 'time' => $line[1]);
        fputcsv($out_file, $data);
      }
    }
  }

  fclose($out_file);
  fclose($file);
}

function drush_eaglecrk_test2() {
  $file = fopen("/home/mdecker/shared/files.csv", "r");

  $files = array();
  while (!feof($file)) {
    $files[] = fgetcsv($file)[0];
  }
  fclose($file);
$i = 1;
  $folders = array();

  foreach ($files as $file) {
    if ($file) {
      drush_print($i);
      $i++;

      $file_name = substr($file, strpos($file, '/files/') + 7);
      $folder = substr($file_name, 0, strrpos($file_name, '/'));
      if(!in_array($folder, $folders)) {
        $folders[] = $folder;
      }

//      if ($folder != '' && !file_exists("/home/mdecker/shared/eoss-files/$folder")) {
//        mkdir("/home/mdecker/shared/eoss-files/$folder", 0777, TRUE);
//      }
//
//      file_put_contents("/home/mdecker/shared/eoss-files/$file_name", fopen($file, 'r'));
    }

  }
sort($folders);
  var_dump($folders);
}


function is_url_exist($url) {
  $ch = curl_init($url);
  curl_setopt($ch, CURLOPT_NOBODY, TRUE);
  curl_exec($ch);
  $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);

  if ($code == 200) {
    $status = TRUE;
  }
  else {
    $status = FALSE;
  }
  curl_close($ch);
  return $status;
}

function drush_eaglecrk_config() {
  $db_user_cache = drush_cache_get('eaglecrk-db-user', 'pantheon-persist')->data;
  $db_pass_cache = drush_cache_get('eaglecrk-db-pass', 'pantheon-persist')->data;
  $mysql_cache = drush_cache_get('eaglecrk-mysql', 'pantheon-persist')->data;
  $git_cache = drush_cache_get('eaglecrk-git', 'pantheon-persist')->data;
  $httpd_conf_cache = drush_cache_get('eaglecrk-httpd-conf', 'pantheon-persist')->data;
  $port_cache = drush_cache_get('eaglecrk-port', 'pantheon-persist')->data;


  $options = array('Lamp', 'WAMP/XAMP', 'Other');
  $local_env = drush_choice($options, 'What is your local environment?');

  drush_print(dt('Enter the directory desired for site files.'));
  $git = str_replace('\\', '/', drush_prompt(dt(''), $git_cache, TRUE));
  while (!file_exists($git)) {
    $git = str_replace('\\', '/', drush_prompt('', $git_cache, TRUE));
  }
  if (substr($git, strlen($git) - 1) == '/') {
    $git = substr($git, 0, strlen($git) - 1);
  }

  while (!is_writable($git)) {
    drush_prompt('This directory is not writeable. Please correct this and press enter', NULL, FALSE);
  }


  switch ($local_env) {
    case 0:
      $db_user = drush_prompt(dt('Username for the database.'), is_null($db_user_cache) ? 'root' : $db_user_cache, FALSE);
      $db_pass = drush_prompt(dt('Password for the database.'), is_null($db_pass_cache) ? '' : $db_pass_cache, FALSE, TRUE);

      $file_proxy = drush_choice(array(
        'No',
        'Yes'
      ), dt('Use "Stage File Proxy" odule'));
      $port = drush_prompt(dt('Port Number'), 80, FALSE, FALSE);
      break;
    case 1:
      $db_user = drush_prompt(dt('Username for the database.'), is_null($db_user_cache) ? 'root' : $db_user_cache, FALSE);
      $db_pass = drush_prompt(dt('Password for the database.'), is_null($db_pass_cache) ? '' : $db_pass_cache, FALSE, TRUE);

      drush_print('Enter the directory for your mysql. This folder must contain a mysql.exe file.');
      $mysql = str_replace('\\', '/', drush_prompt(dt(''), $mysql_cache, TRUE));
      while (!file_exists($mysql . '/mysql.exe')) {
        $mysql = str_replace('\\', '/', drush_prompt('', $mysql_cache, TRUE));
      }
      if (substr($mysql, strlen($mysql) - 1) == '/') {
        $mysql = substr($mysql, 0, strlen($mysql) - 1);
      }
      drush_print('Folder location of apache\'s "httpd-vhosts.conf" file');
      $httpd_conf = str_replace('\\', '/', drush_prompt(dt(''), $httpd_conf_cache, TRUE));
      while (!file_exists($httpd_conf . '/httpd-vhosts.conf')) {
        $httpd_conf = str_replace('\\', '/', drush_prompt('', $httpd_conf_cache, TRUE));
      }
      if (substr($httpd_conf, strlen($httpd_conf) - 1) == '/') {
        $httpd_conf = substr($httpd_conf, 0, strlen($httpd_conf) - 1);
      }
      $file_proxy = drush_choice(array(
        'No',
        'Yes'
      ), dt('Use "Stage File Proxy" module'));
      $port = drush_prompt(dt('Port Number'), is_null($port_cache) ? 80 : $port_cache, FALSE, FALSE);
      break;
    default:
      break;
  }

  drush_cache_set('eaglecrk-db-user', $db_user, 'pantheon-persist');
  drush_cache_set('eaglecrk-db-pass', $db_pass, 'pantheon-persist');
  drush_cache_set('eaglecrk-local-env', $local_env, 'pantheon-persist');
  drush_cache_set('eaglecrk-mysql', $mysql, 'pantheon-persist');
  drush_cache_set('eaglecrk-git', $git, 'pantheon-persist');
  drush_cache_set('eaglecrk-httpd-conf', $httpd_conf, 'pantheon-persist');
  drush_cache_set('eaglecrk-file-proxy', $file_proxy, 'pantheon-persist');
  drush_cache_set('eaglecrk-port', $port, 'pantheon-persist');
}

function drush_eaglecrk_show_config() {
  $db_user = drush_cache_get('eaglecrk-db-user', 'pantheon-persist')->data;
  $db_pass = drush_cache_get('eaglecrk-db-pass', 'pantheon-persist')->data;
  $local_env = drush_cache_get('eaglecrk-local-env', 'pantheon-persist')->data;
  $mysql = drush_cache_get('eaglecrk-mysql', 'pantheon-persist')->data;
  $git = drush_cache_get('eaglecrk-git', 'pantheon-persist')->data;
  $httpd_conf = drush_cache_get('eaglecrk-httpd-conf', 'pantheon-persist')->data;
  $file_proxy = drush_cache_get('eaglecrk-file-proxy', 'pantheon-persist')->data;
  $port = drush_cache_get('eaglecrk-port', 'pantheon-persist')->data;
  $locals = array('Lamp', 'Wamp/Xamp', 'Other');
  $proxy = array('No', 'Yes');

  drush_print("Local Environment: $locals[$local_env]");
  drush_print("File Location: $git");
  if ($local_env == 0 || $local_env == 1) {
    drush_print("DB Username: $db_user");
    drush_print("DB Password: $db_pass");
    drush_print("Use 'Stage File Proxy': $proxy[$file_proxy]");
    drush_print("Port: $port");
  }
  if ($local_env == 1) {
    drush_print("Mysql Location: $mysql");
    drush_print("httpd-vhost.conf: $httpd_conf");
  }
}

function drush_eaglecrk_clone_site($site_name = FALSE, $environment = 'live') {
  if (is_null(drush_cache_get('eaglecrk-local-env', 'pantheon-persist')->data)) {
    drush_eaglecrk_config();
  }
  $local_env = drush_cache_get('eaglecrk-local-env', 'pantheon-persist')->data;
  $git = drush_cache_get('eaglecrk-git', 'pantheon-persist')->data;
  $db_user = drush_cache_get('eaglecrk-db-user', 'pantheon-persist')->data;
  $db_pass = drush_cache_get('eaglecrk-db-pass', 'pantheon-persist')->data;
  $port = drush_cache_get('eaglecrk-port', 'pantheon-persist')->data;

  $site_uuid = FALSE;
  while (!$site_uuid) {
    $site_uuid = terminus_get_site_uuid_by_name($site_name);
    if (is_null($site_name) || !$site_uuid) {
      $site_name = drush_prompt('Name of site', NULL, TRUE);
    }
  }

  $bucket = 'latest';
  $element = 'database';

  $site_folder = drush_prompt('Folder to clone into', $site_name, FALSE);

  switch ($local_env) {
    case 0:
      $new_db = drush_prompt('Database Name', NULL, TRUE, FALSE);
      $new_db = strtolower(preg_replace('/[^a-zA-Z0-9]/', '_', $new_db));

      $sites_url = drush_prompt('Local Url to use for this site', NULL, TRUE, FALSE);
      $sites_url = strtolower(preg_replace('/[^a-zA-Z0-9\.]/', '-', $sites_url));

      if (drush_cache_get('eaglecrk-file-proxy', 'pantheon-persist')->data) {
        $orig_url = drush_prompt(dt('Original URL of site (for Stage File Proxy)'), NULL, TRUE, FALSE);
        $orig_url = trim($orig_url, "/\\");
      }

      $git_clone = 'git clone ssh://codeserver.dev.' . $site_uuid . '@codeserver.dev.' . $site_uuid . '.drush.in:2222/~/repository.git ' . $git . '/' . $site_folder;
      shell_exec($git_clone);
      if (!file_exists("$git/$site_folder/sites/default/settings.php") && file_exists("$git/$site_folder/sites/default/default.settings.php")) {
        shell_exec("cp $git/$site_folder/sites/default/default.settings.php $git/$site_folder/sites/default/settings.php");
      }
      else {
        if (!file_exists("$git/$site_folder/sites/default/settings.php")) {
          shell_exec("touch $git/$site_folder/sites/default/settings.php");
        }
      }

      shell_exec('sudo chmod 777 ' . $git . '/' . $site_folder . '/sites/default/settings.php');

      $add_string = "\$databases = array(\n\t'default' => array(\n\t\t'default' => array(";
      $add_string .= "\n\t\t\t'driver' => 'mysql',\n\t\t\t'database' => '" . $new_db . "',";
      $add_string .= "\n\t\t\t'username' => '" . $db_user . "',\n\t\t\t'password' => '" . $db_pass . "',";
      $add_string .= "\n\t\t\t'host' => 'localhost',\n\t\t\t'prefix' => '',";
      $add_string .= "\n\t\t\t'collation' => 'utf8_general_ci',\n\t\t)\n\t) \n);\n";
      $add_string .= "\n\$base_url = $sites_url;";
      file_put_contents($git . '/' . $site_folder . '/sites/default/settings.php', $add_string, FILE_APPEND);

      shell_exec('sudo chmod 777 /etc/hosts');
      if (file_exists('/etc/hosts')) {
        file_put_contents('/etc/hosts', "\n127.0.0.1\t" . $sites_url, FILE_APPEND);
      }

      $db_file = drush_eaglecrk_pantheon_site_download_backup($site_uuid, $environment, $bucket, $element, $git);
      if (is_null($db_file) || $db_file == '') {
        drush_terminus_pantheon_site_make_backup($site_uuid, $environment);
        drush_print('Backup Initiated. Please Wait');
        while (is_null($db_file) || $db_file == '') {
          sleep(15);
          $db_file = drush_eaglecrk_pantheon_site_download_backup($site_uuid, $environment, $bucket, $element, $git);
        }
      }


      shell_exec("gzip -d " . $git . '/' . $db_file);
      $db_file = $git . '/' . str_replace('.gz', '', $db_file);
      drush_print('Database file downloaded and extracted.');

      if ($db_pass == '' || is_null($db_pass)) {
        shell_exec('mysql -u ' . $db_user . ' -e "create database ' . $new_db . ';"');
        shell_exec('mysql -u ' . $db_user . ' ' . $new_db . ' < ' . $db_file);
      }
      else {
        shell_exec('mysql -u ' . $db_user . '-p' . $db_pass . ' -e "create database ' . $new_db . ';"');
        shell_exec('mysql -u ' . $db_user . ' -p' . $db_pass . ' ' . $new_db . ' < ' . $db_file);
      }
      drush_print('Database Imported');

      shell_exec("sudo touch /etc/apache2/sites-available/" . $sites_url . '.conf');
      shell_exec("sudo chmod 777 /etc/apache2/sites-available/" . $sites_url . '.conf');

      $vhost = "<VirtualHost *:$port>\n\tServerName $sites_url \n\tDocumentRoot $git/$site_folder\n";
      $vhost .= "ErrorLog \"/var/www/$site_folder errorlog.log\"";
      $vhost .= "\t<Directory \"$git/$site_folder\">\n\t\tAllowOverride All\n\t</Directory>\n</VirtualHost>";
      file_put_contents('/etc/apache2/sites-available/' . $sites_url . '.conf', $vhost);
      shell_exec('sudo a2ensite ' . $sites_url . '.conf');
      shell_exec('sudo service apache2 reload');
      drush_print('Hosts file and vitual host configured');


      if (drush_cache_get('eaglecrk-file-proxy', 'pantheon-persist')->data) {
        shell_exec("sudo chmod a+w $git/$site_folder/sites/default");
        shell_exec("cd $git/$site_folder && drush dl --destination=sites/default/modules stage_file_proxy -y");
        shell_exec("cd $git/$site_folder && drush en stage_file_proxy -y");
        $add_string = "\n\$conf['stage_file_proxy_origin'] = '$orig_url';\n";
        file_put_contents($git . '/' . $site_folder . '/sites/default/settings.php', $add_string, FILE_APPEND);
        drush_print('Stage File Proxy Module Enabled');
      }

      if (!file_exists("$git/$site_folder/sites/default/files/temp")) {
        shell_exec("mkdir $git/$site_folder/sites/default/files/temp");
      }
      $modules = shell_exec('drush pml --format=var_export');
      if (isset($modules['views_cache_bully']) && $modules['views_cache_bully']['status'] == 'Enabled') {
        shell_exec("cd $git/$site_folder && drush vset views_cache_bully_output_lifespan -1 --yes; cd $git/$site_folder && drush vset views_cache_bully_results_lifespan -1 --yes;");
      }
      shell_exec("cd $git/$site_folder && drush vset preprocess_js 0 --yes; cd $git/$site_folder && drush vset preprocess_css 0 --yes; drush vset file_temporary_path 'sites/default/files/temp'; cd $git/$site_folder && drush cc all; cd $git/$site_folder && drush vset cron_safe_threshold 0");

      shell_exec("sudo chmod 444 $git/$site_folder/sites/default/settings.php");

      if (!file_exists("$git/$site_folder/sites/default/files")) {
        shell_exec("mkdir $git/$site_folder/sites/default/files");
      }

      shell_exec("sudo chown -R www-data:www-data $git/$site_folder/sites/default/files");
      shell_exec("sudo chmod -R 0770 $git/$site_folder/sites/default/files");

      shell_exec('sudo rm ' . $db_file);
      drush_print("Clone Complete. Visit http://$sites_url" . ($port != 80 ? ":$port" : '') . " to view the site.");
      break;

    case 1:
      $new_db = drush_prompt('Database Name', NULL, TRUE, FALSE);
      $new_db = strtolower(preg_replace('/[^a-zA-Z0-9]/', '_', $new_db));

      $sites_url = drush_prompt('Local Url to use for this site', NULL, TRUE, FALSE);
      $sites_url = strtolower(preg_replace('/[^a-zA-Z0-9\.]/', '-', $sites_url));

      if (drush_cache_get('eaglecrk-file-proxy', 'pantheon-persist')->data) {
        $orig_url = drush_prompt(dt('Original URL of site (for Stage File Proxy)'), NULL, TRUE, FALSE);
      }

      $git_clone = "git clone ssh://codeserver.dev.$site_uuid@codeserver.dev.$site_uuid.drush.in:2222/~/repository.git $git/$site_folder";
      shell_exec($git_clone);

      $add_string = "\$databases = array(\n\t'default' => array(\n\t\t'default' => array(";
      $add_string .= "\n\t\t\t'driver' => 'mysql',\n\t\t\t'database' => '$new_db',";
      $add_string .= "\n\t\t\t'username' => '$db_user',\n\t\t\t'password' => '$db_pass',";
      $add_string .= "\n\t\t\t'host' => 'localhost',\n\t\t\t'prefix' => '',";
      $add_string .= "\n\t\t\t'collation' => 'utf8_general_ci',\n\t\t)\n\t) \n);\n";
      $add_string .= "\n\$base_url = 'http://$sites_url';";

      $settings_file = $git . '/' . $site_folder . '/sites/default/settings.php';
      if (!file_exists($settings_file)) {
        copy($git . '/' . $site_folder . '/sites/default/default.settings.php', $settings_file);
      }

      file_put_contents($settings_file, $add_string, FILE_APPEND);
      if (file_exists('C:/Windows/System32/drivers/etc/hosts')) {
        file_put_contents('C:/Windows/System32/drivers/etc/hosts', "\n127.0.0.1\t" . $sites_url, FILE_APPEND);
      }

      $mysql = drush_cache_get('eaglecrk-mysql', 'pantheon-persist')->data;
      $db_file = drush_eaglecrk_pantheon_site_download_backup($site_uuid, $environment, $bucket, $element, $mysql);
      if (is_null($db_file) || $db_file == '') {
        drush_terminus_pantheon_site_make_backup($site_uuid, $environment);
        drush_print('Please Wait...');
        while (is_null($db_file) || $db_file == '') {
          sleep(25);
          $db_file = drush_eaglecrk_pantheon_site_download_backup($site_uuid, $environment, $bucket, $element, $mysql);
        }
      }
      $db_file = $mysql . '/' . $db_file;

      $buffer_size = 4096;
      $file = gzopen($db_file, 'rb');
      $out_file = fopen(str_replace('.gz', '', $db_file), 'wb');
      while (!gzeof($file)) {
        fwrite($out_file, gzread($file, $buffer_size));
      }
      fclose($out_file);
      gzclose($file);
      unlink($db_file);
      $db_file = str_replace('.gz', '', $db_file);

      if ($db_pass == '' || is_null($db_pass)) {
        shell_exec($mysql . '/mysql.exe -u ' . $db_user . ' -e "create database ' . $new_db . '";');
        shell_exec($mysql . '/mysql.exe -u ' . $db_user . ' ' . $new_db . ' < ' . $db_file);
      }
      else {
        shell_exec($mysql . '/mysql.exe -u ' . $db_user . '-p' . $db_pass . ' -e "create database ' . $new_db . '";');
        shell_exec($mysql . '/mysql.exe -u ' . $db_user . ' -p' . $db_pass . ' ' . $new_db . ' < ' . $db_file);
      }
      unlink($db_file);


      $httpd_conf = drush_cache_get('eaglecrk-httpd-conf', 'pantheon-persist')->data;
      if (file_exists($httpd_conf . '/httpd-vhosts.conf')) {
        $vhost = "\n\n<VirtualHost *:$port>\n\tDocumentRoot \"$git/$site_folder\"\n\tServerName $sites_url\n</VirtualHost>";
        file_put_contents($httpd_conf . '\httpd-vhosts.conf', $vhost, FILE_APPEND);
      }

      if (drush_cache_get('eaglecrk-file-proxy', 'pantheon-persist')->data) {
        shell_exec("cd $git/$site_folder && drush dl --destination=sites/default/modules stage_file_proxy -y");
        shell_exec("cd $git/$site_folder && drush en stage_file_proxy -y");
        $add_string = "\n\$conf['stage_file_proxy_origin'] = '$orig_url';\n";
        file_put_contents($git . '/' . $site_folder . '/sites/default/settings.php', $add_string, FILE_APPEND);
      }

      drush_print("Clone Complete. Visit http://$sites_url" . ($port != 80 ? ":$port" : '') . " to view the site after restarting your apache service.");
      break;

    default:
      $db_file = drush_eaglecrk_pantheon_site_download_backup($site_uuid, $environment, $bucket, $element, $git);
      if (is_null($db_file) || $db_file == '') {
        drush_terminus_pantheon_site_make_backup($site_uuid, $environment);
        drush_print('Please Wait...');
        while (is_null($db_file) || $db_file == '') {
          sleep(20);
          $db_file = drush_eaglecrk_pantheon_site_download_backup($site_uuid, $environment, $bucket, $element, $git);
        }
      }
      $db_file = $git . '/' . $db_file;

      $buffer_size = 4096;
      $file = gzopen($db_file, 'rb');
      $out_file = fopen(str_replace('.gz', '', $db_file), 'wb');
      while (!gzeof($file)) {
        fwrite($out_file, gzread($file, $buffer_size));
      }
      fclose($out_file);
      gzclose($file);
      unlink($db_file);

      $git_clone = 'git clone ssh://codeserver.dev.' . $site_uuid . '@codeserver.dev.' . $site_uuid . '.drush.in:2222/~/repository.git ' . $git . '/' . $site_folder;
      shell_exec($git_clone);

      drush_print('Database and files downloaded to ' . $git);
      break;
  }
}


/**
 * Download a backup from a site.
 */
function drush_eaglecrk_pantheon_site_download_backup($site_uuid = FALSE, $environment = FALSE, $bucket = FALSE, $element = FALSE, $mysql = FALSE) {
  $session_data = terminus_bootstrap();
  if ($session_data === FALSE) {
    return FALSE;
  }
  extract($session_data);

  if (!$site_uuid = terminus_site_input($site_uuid)) {
    drush_log('You must supply a site UUID', 'failed');
    return FALSE;
  }

  if (!$environment) {
    $environment = terminus_session_select_data('environment', $site_uuid);
    if (!$environment) {
      drush_log('You must supply an environment', 'failed');
      return FALSE;
    }
  }
  if (!$element) {
    $element = terminus_session_select_data('element');
    if (!$element) {
      drush_log('You must supply an element', 'failed');
      return FALSE;
    }
  }

  // Retrieve the latest bucket
  if ($bucket == 'latest') {
    $bucket = terminus_latest_bucket($site_uuid, $environment, $element);
  }
  if (!$bucket) {
    $bucket = terminus_session_select_data('bucket', $site_uuid, $environment, $element);
    if (!$bucket) {
      drush_log('You must supply a bucket', 'failed');
      return FALSE;
    }
  }
  if (!$mysql) {
    $mysql = terminus_session_select_data('destination');
    if (!$mysql) {
      drush_log('You must supply a destination', 'failed');
      return FALSE;
    }
  }

  $result = terminus_api_backup_download_url($site_uuid, $environment, $bucket, $element);
  $data = json_decode($result['json']);
  $filename = strstr(basename($data->url), '?', '_');

  drush_log("Downloading " . $filename . "...");

  // Copied from pm code
  $cache_duration = 86400 * 365;
  $path = _drush_download_file($data->url, $mysql . DIRECTORY_SEPARATOR . $filename, $cache_duration);
  if ($path || drush_get_context('DRUSH_SIMULATE')) {
    drush_log("Downloading " . $filename . " was successful.");
  }
  else {
    return drush_set_error('DRUSH_PSITE_DOWNLOAD_FAILED', 'Unable to download ' . $filename . ' to ' . $mysql . ' from ' . $data->url);
  }

  return $filename;
}


function drush_eaglecrk_restore_database($site_name = NULL, $environment = 'live') {
  if (is_null(drush_cache_get('eaglecrk-local-env', 'pantheon-persist')->data)) {
    drush_eaglecrk_config();
  }
  $local_env = drush_cache_get('eaglecrk-local-env', 'pantheon-persist')->data;
  $git = drush_cache_get('eaglecrk-git', 'pantheon-persist')->data;
  $db_user = drush_cache_get('eaglecrk-db-user', 'pantheon-persist')->data;
  $db_pass = drush_cache_get('eaglecrk-db-pass', 'pantheon-persist')->data;

  $site_uuid = FALSE;
  while (!$site_uuid) {
    $site_uuid = terminus_get_site_uuid_by_name($site_name);
    if (is_null($site_name) || !$site_uuid) {
      $site_name = drush_prompt('Name of site', NULL, TRUE);
    }
  }

  $bucket = 'latest';
  $element = 'database';


  switch ($local_env) {
    case 0:
      $drush = shell_exec('drush status --format=json');
      $drush = json_decode($drush, TRUE);

      if (!isset($drush['db-name'])) {
        $new_db = drush_prompt('Database to overwrite', NULL, TRUE, FALSE);
        $new_db = strtolower(preg_replace('/[^a-zA-Z0-9]/', '_', $new_db));
        $in_drupal = FALSE;
      }
      else {
        $new_db = $drush['db-name'];
        $in_drupal = TRUE;
      }

      $db_file = drush_eaglecrk_pantheon_site_download_backup($site_uuid, $environment, $bucket, $element, $git);
      shell_exec("gzip -d " . $git . '/' . $db_file);

      $db_file = $git . '/' . str_replace('.gz', '', $db_file);

      if ($db_pass == '' || is_null($db_pass)) {
        shell_exec('mysql -u ' . $db_user . ' -e "drop database ' . $new_db . ';"');
        shell_exec('mysql -u ' . $db_user . ' -e "create database ' . $new_db . ';"');
        shell_exec('mysql -u ' . $db_user . ' ' . $new_db . ' < ' . $db_file);
      }
      else {
        shell_exec('mysql -u ' . $db_user . '-p' . $db_pass . ' -e "drop database ' . $new_db . ';"');
        shell_exec('mysql -u ' . $db_user . '-p' . $db_pass . ' -e "create database ' . $new_db . ';"');
        shell_exec('mysql -u ' . $db_user . ' -p' . $db_pass . ' ' . $new_db . ' < ' . $db_file);
      }

      shell_exec('rm ' . $db_file);
      if ($in_drupal) {
        shell_exec('drush vset preprocess_js 0 --yes; drush vset preprocess_css 0 --yes; drush vset file_temporary_path "sites/default/files/temp"; drush vset cron_safe_threshold 0');
        $modules = shell_exec('drush pml --format=var_export');
        if (isset($modules['views_cache_bully']) && $modules['views_cache_bully']['status'] == 'Enabled') {
          shell_exec("cd $git/$site_folder && drush vset views_cache_bully_output_lifespan -1 --yes; cd $git/$site_folder && drush vset views_cache_bully_results_lifespan -1 --yes;");
        }
      }

      break;
    case 1:
      $drush = shell_exec('drush status');
      $drush = explode("\n", $drush);
      foreach ($drush as &$line) {
        $line = explode(':', $line);
      }
      $status = array();
      foreach ($drush as $line) {
        if (isset($line[0]) && isset($line[1])) {
          $status[trim($line[0])] = trim($line[1]);
        }
      }

      if (!isset($status['Database name'])) {
        $new_db = drush_prompt('Database to overwrite', NULL, TRUE, FALSE);
        $new_db = strtolower(preg_replace('/[^a-zA-Z0-9]/', '_', $new_db));
      }
      else {
        $new_db = $status['Database name'];
      }

      $mysql = drush_cache_get('eaglecrk-mysql', 'pantheon-persist')->data;
      $db_file = drush_eaglecrk_pantheon_site_download_backup($site_uuid, $environment, $bucket, $element, $mysql);


      $buffer_size = 4096;
      $file = gzopen($db_file, 'rb');
      $out_file = fopen(str_replace('.gz', '', $db_file), 'wb');
      while (!gzeof($file)) {
        fwrite($out_file, gzread($file, $buffer_size));
      }
      fclose($out_file);
      gzclose($file);


      unlink($db_file);
      $db_file = str_replace('.gz', '', $db_file);

      if ($db_pass == '' || is_null($db_pass)) {
        shell_exec($mysql . '/mysql.exe -u ' . $db_user . ' -e "drop database ' . $new_db . '";');
        shell_exec($mysql . '/mysql.exe -u ' . $db_user . ' -e "create database ' . $new_db . '";');
        shell_exec($mysql . '/mysql.exe -u ' . $db_user . ' ' . $new_db . ' < ' . $db_file);
      }
      else {
        shell_exec($mysql . '/mysql.exe -u ' . $db_user . '-p' . $db_pass . ' -e "drop database ' . $new_db . '";');
        shell_exec($mysql . '/mysql.exe -u ' . $db_user . '-p' . $db_pass . ' -e "create database ' . $new_db . '";');
        shell_exec($mysql . '/mysql.exe -u ' . $db_user . ' -p' . $db_pass . ' ' . $new_db . ' < ' . $db_file);
      }
      unlink($db_file);


      break;
    default:
      drush_print('You do not have a compatible environment for this command.');
      break;
  }

  $drush = shell_exec('drush status');
  $drush = explode("\n", $drush);
  foreach ($drush as &$line) {
    $line = explode(':', $line);
  }
  $status = array();
  foreach ($drush as $line) {
    if (isset($line[0]) && isset($line[1])) {
      $status[trim($line[0])] = trim($line[1]);
    }
  }

  if (!isset($status['Database name'])) {
    shell_exec('drush vset preprocess_css 0 --yes; drush vset preprocess_js 0 --yes; drush cc all; cd $git/$site_folder && drush vset cron_safe_threshold 0');
  }

}

function drush_eaglecrk_clear_terminus() {
  $local_env = drush_cache_get('eaglecrk-local-env', 'pantheon-persist')->data;
  switch ($local_env) {
    case 0:
      $file = shell_exec("[ -f \$HOME/.drush/cache/pantheon-persist/terminus-last-email.cache ] && echo 1 || echo 0");
      if ($file) {
        shell_exec("rm \$HOME/.drush/cache/pantheon-persist/terminus-last-email.cache");
      }

      $file = shell_exec("[ -f \$HOME/.drush/cache/pantheon-persist/eaglecrk-db-pass.cache ] && echo 1 || echo 0");
      if ($file) {
        shell_exec("rm \$HOME/.drush/cache/pantheon-persist/eaglecrk-db-pass.cache");
      }

      $file = shell_exec("[ -f \$HOME/.drush/cache/pantheon-persist/eaglecrk-db-user.cache ] && echo 1 || echo 0");
      if ($file) {
        shell_exec("rm \$HOME/.drush/cache/pantheon-persist/eaglecrk-db-user.cache");
      }

      $file = shell_exec("[ -f \$HOME/.drush/cache/pantheon-persist/eaglecrk-file-proxy.cache ] && echo 1 || echo 0");
      if ($file) {
        shell_exec("rm \$HOME/.drush/cache/pantheon-persist/eaglecrk-file-proxy.cache");
      }

      $file = shell_exec("[ -f \$HOME/.drush/cache/pantheon-persist/eaglecrk-git.cache ] && echo 1 || echo 0");
      if ($file) {
        shell_exec("rm \$HOME/.drush/cache/pantheon-persist/eaglecrk-git.cache");
      }

      $file = shell_exec("[ -f \$HOME/.drush/cache/pantheon-persist/eaglecrk-httpd-conf.cache ] && echo 1 || echo 0");
      if ($file) {
        shell_exec("rm \$HOME/.drush/cache/pantheon-persist/eaglecrk-httpd-conf.cache");
      }

      $file = shell_exec("[ -f \$HOME/.drush/cache/pantheon-persist/eaglecrk-local-env.cache ] && echo 1 || echo 0");
      if ($file) {
        shell_exec("rm \$HOME/.drush/cache/pantheon-persist/eaglecrk-local-env.cache");
      }

      $file = shell_exec("[ -f \$HOME/.drush/cache/pantheon-persist/eaglecrk-mysql.cache ] && echo 1 || echo 0");
      if ($file) {
        shell_exec("rm \$HOME/.drush/cache/pantheon-persist/eaglecrk-mysql.cache");
      }

      $file = shell_exec("[ -f \$HOME/.drush/cache/pantheon-persist/eaglecrk-port.cache ] && echo 1 || echo 0");
      if ($file) {
        shell_exec("rm \$HOME/.drush/cache/pantheon-persist/eaglecrk-port.cache");
      }

      break;
    default:
      drush_print('Incompatible with your environment. Please manually delete the files in $HOME/.drush/cache/pantheon-persist');
      break;
  }
}

function drush_eaglecrk_create_site($name = FALSE) {


  $session_data = drush_cache_get('terminus-current-session', 'pantheon');
  $user_uuid = $session_data->data['user_uuid'];
  $git = drush_cache_get('eaglecrk-git', 'pantheon-persist')->data;


  $label = drush_prompt('Human readable label for the site', NULL, TRUE);
  $organization = 'af1c6817-849b-4fd8-9b29-01274675791e';
  $name = drush_prompt('Machine name of the site; used as part of the default URL', strtolower(preg_replace('/[^a-zA-Z0-9\.]/', '-', $label)), TRUE);
  $product = '21e1fada-199c-492b-97bd-0b36b53a9da0';

  drush_log(dt('Queuing build.'), 'ok');
  $result_create = terminus_api_user_site_create($user_uuid, $name, $label, $organization, $product);
  if (!$result_create) {
    return drush_set_error('DRUSH_PSITE_CREATE_FAILED', dt('Site creation failed.'));
  }
  if (isset($json_output) && isset($result_create['json']) && $result_create['json']) {
    drush_print($result_create['json']);
    return TRUE;
  }

  $result = json_decode($result_create['json']);
  $site_uuid = $result->site_id;
  drush_log(dt('Your new site "@name" is now building with UUID "@site_uuid".', array(
    '@name' => $name,
    '@site_uuid' => $site_uuid,
  )), 'ok');
  drush_cache_clear_all('terminus-sites', 'pantheon', TRUE);
  drush_cache_clear_all('terminus-user-sites-' . $user_uuid, 'pantheon', TRUE);
  if ($organization) {
    drush_cache_clear_all('terminus-org-sites-' . $organization, 'pantheon', TRUE);
  }

  // Poll for completion.
  if (drush_get_option('nopoll')) {
    drush_log(dt('Not polling for completion; check your dashboard to see when this site is ready.'));
    return TRUE;
  }
  drush_print(dt('Waiting for site spin-up to complete. Do Not Cancel This Process!'));


  sleep(15);
  while (shell_exec("git ls-remote ssh://codeserver.dev.$site_uuid@codeserver.dev.$site_uuid.drush.in:2222/~/repository.git") == NULL) {
    sleep(15);
  }

  if (!is_null($git)) {
    $git_clone = "git clone ssh://codeserver.dev.$site_uuid@codeserver.dev.$site_uuid.drush.in:2222/~/repository.git $git/$name";
    shell_exec($git_clone);
    shell_exec("drush psite-cmode $name dev 'git'");
    shell_exec("cd $git/$name && git pull https://github.com/ec-mdecker/ec-modules.git master");
    shell_exec("cd $git/$name && git push origin master");
  }
  else {
    $git_clone = "git clone ssh://codeserver.dev.$site_uuid@codeserver.dev.$site_uuid.drush.in:2222/~/repository.git \$HOME/$name";
    shell_exec($git_clone);
    shell_exec("drush psite-cmode $name dev 'git'");
    shell_exec("cd \$HOME/$name && git pull https://github.com/ec-mdecker/ec-modules.git master");
    shell_exec("cd \$HOME/$name && git push origin master");
  }


  drush_print('https://dashboard.getpantheon.com/sites/' . $site_uuid);
}
