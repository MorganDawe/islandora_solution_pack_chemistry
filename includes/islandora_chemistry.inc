<?php

class IslandoraOBabel {
  protected $islandoraObject;

  public function __construct($pid) {
    module_load_include('inc', 'islandora', 'includes/utilities');
    $this->islandoraObject = islandora_object_load($pid);
  }

  /**
   * Function to create a standard MOL file from whatever format is ingested.
   *
   * @param type $dsid
   * @param type $file
   * @param type $file_ext
   * @return string
   */
  public static function chem2mol($dsid, $file, $file_ext) {
    module_load_include('inc', 'islandora_chemistry', 'includes/FileConversion');
    $file_suffix = '-' . $dsid . '.' . $file_ext;
    $returnValue = TRUE;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);
    clean_XYZ($file);

//  Call openbabel and convert the file to MOL. (No longer needed - Create the 3D structure so that it looks good in the 3D viewer)
    if (!file_exists($file . "svg")) {
      system("obabel \"$file\" --title -omol -xw -O\"$filename\"", $returnValue);
    }
    else {
      $returnValue = '0';
    }
    $returnValue = '0';
    if ($returnValue == '0') {
      // Session var's for use in batch?
      $_SESSION['fedora_ingest_files']["$dsid"] = $filename;
      return TRUE;
    }
    else {
      return $returnValue;
    }
  }

 /**
   * Function to generate data from structure file and create an XML file.
   *
   * @param type $dsid
   * @param type $file
   * @param type $file_ext
   * @return string
   */
  public static function chem2cml($dsid, $file, $file_ext, $separate = TRUE) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'islandora_chemistry', 'FileConversion');
    $file_suffix = '_' . $dsid . '.' . $file_ext;
    $returnValue = TRUE;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);
//    drupal_set_message('File: ' . $file);
    clean_XYZ($file);
//  Run various system calls to generate the data
    if (!file_exists($filename)) {
      exec("obprop \"$file\"", $return, $returnValue1);
      if ($returnValue1 != '1') {
        drupal_set_message(t('obprop failed to run properly'), 'error');
        return $returnValue1;
      }
      if ($separate) {
      exec("obabel \"$file\" -oinchi --separate -xK", $inchi_key, $returnValue2);
      }
      else {
        exec("obabel \"$file\" -oinchi -xK", $inchi_key, $returnValue2);
      }
      if ($returnValue2 != '0') {
        drupal_set_message(t('The InChIKey could not be calculated'), 'error');
        return $returnValue2;
      }
      exec("obabel \"$file\" -omol -O/tmp/checkmol.mol", $checkreturn, $returnValue3);
      if ($returnValue3 != '0') {
        drupal_set_message(t('The MOL file could not be generated'), 'error');
        return $returnValue3;
      }
      exec("checkmol -c /tmp/checkmol.mol", $fragment, $returnValue4);
      switch ($returnValue4) {
        case 2:
          drupal_set_message(t('checkmol could not find the specified file'), 'warning');
          break;
        case 3:
          drupal_set_message(t('checkmol did not recognise the file type'), 'warning');
          break;
      }

      if ($returnValue4 != 0) {
        $fragment = NULL;
      }

      exec("obabel \"$file\" -osmi -O /tmp/checkmol.smi -xc", $return, $returnValue5);
      if ($returnValue5 != '0') {
        drupal_set_message(t('The smiles string could not be calculated'), 'error');
        return $returnValue5;
      }
      ///opt/ActivePython-2.7/bin/python2.7
      $cmd = '/usr/bin/python2.7 fingerprint.py /tmp/checkmol.smi';
      dsm($cmd, "command");
//      This calls a python script to generate the molecular fingerprint
      exec($cmd, $fingerprint, $returnValue6);
      dsm($returnValue6, "ret val");
      dsm($fingerprint, "fingerprint");
      if ($returnValue6 != '0') {
        drupal_set_message(t('The molecular fingerprint could not be calculated'), 'error');
        return $returnValue6;
      }

// //      The python script leaves some data in the string it generates so this is stripped out
//       $fingerprint = explode(', ', str_replace(array('[', ']', 'L'), array('', '', ''), $fingerprint[0]));
// //      The returned arrays are parsed to obtain the relevant data
//       $name = $return[0];
//       $name = preg_replace('!\s+!', ' ', $name);
//       $name = str_replace('"', '', $name);
//       $name = ltrim($name, 'name ');
//       $mf = $return[1];
//       $mf = preg_replace('!\s+!', ' ', $mf);
//       $mf_array = explode(' ', $mf);
//       $mf_value = $mf_array[1];
//       $mf_value = preg_replace('/([A-Z][a-z]?)/', '$1 ', $mf_value);
//       $mf_value = preg_replace('/([0-9]*)/', '$1 ', $mf_value);
//       $mf_value = preg_replace('!\s+!', ' ', $mf_value);
//       $mf_value = trim($mf_value);
//       $mw = $return[2];
//       $mw = preg_replace('!\s+!', ' ', $mw);
//       $mw_array = explode(' ', $mw);
//       $mw_value = $mw_array[1];
//       $inchi = $return[6];
//       $inchi = preg_replace('!\s+!', ' ', $inchi);
//       $inchi_array = explode(' ', $inchi);
//       $inchi_value = $inchi_array[1];
// //      The chemspider API is queried to retrieve the IUPAC name of the compound

//       $rdf = new DOMDocument();
//       $rdf_url = 'http://rdf.chemspider.com/' . htmlentities($inchi_key[0]);

//       if (islandora_chem_sp_url_exists('http://rdf.chemspider.com/')) {

//         $url_headers = get_headers($rdf_url);

//         if ($url_headers[0] == 'HTTP/1.1 302 Found') {
//           $rdf->load($rdf_url);
//           $trivial = $rdf->getElementsByTagName('Description')->item(13)->nodeValue;
// //    The URL to the chemspider entry is also generated
//           if ($rdf->getElementsByTagName('Description')->item(1)) {
//             $spider_url = $rdf->getElementsByTagName('Description')->item(1)->getAttribute('rdf:about');
//           }
//           else {
//             $trivial = 'No synonyms available';
//           }
//         }
//         else {
//           $trivial = 'Structure not found';
//           $spider_url = '';
//         }
//       }
//       else {
//         $trivial = 'Structure not found';
//         drupal_set_message(t('Could not connect to http://rdf.chemspider.com/'), 'warning');
//       }

//       $fragment = trim(str_replace(';', ' ', $fragment[0]));
//       $smiles = file_get_contents('/tmp/checkmol.smi');
//       $smiles = explode("\t", $smiles);

// //    The molecular fingerprint consists of 117 values which are put into a MySQL db
// //    This allows for substructure searching using a bitwise comparison
//       for ($i = 0; $i < 117; $i++) {
//         ${fpv2 . $i} = $fingerprint[$i];
//       }
// //    A check is done initially to see if this molecule has been stored before
//       $query2 = db_query("SELECT id FROM {islandora_chem_fingerprints} WHERE inchikey = '%s'", $inchi_key[0]);

//       $test2 = NULL;

//       while ($obj2 = db_fetch_object($query2)) {
//         $test2 = $obj2->id;
//       }

//       if (!isset($test2)) {
//         $result = db_query("INSERT INTO {islandora_chem_fingerprints} (inchikey, fp0, fp1, fp2, fp3, fp4, fp5, fp6, fp7, fp8, fp9, fp10, fp11, fp12, fp13, fp14, fp15, fp16, fp17, fp18, fp19, fp20, fp21, fp22, fp23, fp24, fp25, fp26, fp27, fp28, fp29, fp30, fp31, fp32, fp33, fp34, fp35, fp36, fp37, fp38, fp39, fp40, fp41, fp42, fp43, fp44, fp45, fp46, fp47, fp48, fp49, fp50, fp51, fp52, fp53, fp54, fp55, fp56, fp57, fp58, fp59, fp60, fp61, fp62, fp63, fp64, fp65, fp66, fp67, fp68, fp69, fp70, fp71, fp72, fp73, fp74, fp75, fp76, fp77, fp78, fp79, fp80, fp81, fp82, fp83, fp84, fp85, fp86, fp87, fp88, fp89, fp90, fp91, fp92, fp93, fp94, fp95, fp96, fp97, fp98, fp99, fp100, fp101, fp102, fp103, fp104, fp105, fp106, fp107, fp108, fp109, fp110, fp111, fp112, fp113, fp114, fp115, fp116) VALUES ('$inchi_key[0]', $fpv20, $fpv21, $fpv22, $fpv23, $fpv24, $fpv25, $fpv26, $fpv27, $fpv28, $fpv29, $fpv210, $fpv211, $fpv212, $fpv213, $fpv214, $fpv215, $fpv216, $fpv217, $fpv218, $fpv219, $fpv220, $fpv221, $fpv222, $fpv223, $fpv224, $fpv225, $fpv226, $fpv227, $fpv228, $fpv229, $fpv230, $fpv231, $fpv232, $fpv233, $fpv234, $fpv235, $fpv236, $fpv237, $fpv238, $fpv239, $fpv240, $fpv241, $fpv242, $fpv243, $fpv244, $fpv245, $fpv246, $fpv247, $fpv248, $fpv249, $fpv250, $fpv251, $fpv252, $fpv253, $fpv254, $fpv255, $fpv256, $fpv257, $fpv258, $fpv259, $fpv260, $fpv261, $fpv262, $fpv263, $fpv264, $fpv265, $fpv266, $fpv267, $fpv268, $fpv269, $fpv270, $fpv271, $fpv272, $fpv273, $fpv274, $fpv275, $fpv276, $fpv277, $fpv278, $fpv279, $fpv280, $fpv281, $fpv282, $fpv283, $fpv284, $fpv285, $fpv286, $fpv287, $fpv288, $fpv289, $fpv290, $fpv291, $fpv292, $fpv293, $fpv294, $fpv295, $fpv296, $fpv297, $fpv298, $fpv299, $fpv2100, $fpv2101, $fpv2102, $fpv2103, $fpv2104, $fpv2105, $fpv2106, $fpv2107, $fpv2108, $fpv2109, $fpv2110, $fpv2111, $fpv2112, $fpv2113, $fpv2114, $fpv2115, $fpv2116)");
//         if ($result == FALSE) {
//           drupal_set_message(t('Database update failed!', 'warning'));
//         }
//       }
// //      The XML is created and populated with the data obtained above
//       $dom = new DOMDocument();

//       $cml = $dom->createElementNS("http://www.xml-cml.org/schema", "cml:cml");
//       $dom->appendChild($cml);

//       $xmlname = $dom->createElement("molecule");
//       $xmlname->setAttribute("title", $name);
//       $cml->appendChild($xmlname);

//       $xmltrivial = $dom->createElement("name", ucfirst(ltrim($trivial)));
//       $xmltrivial->setAttribute("id", "n0");
//       $xmltrivial->setAttribute("convention", "IUPAC:trivial");
//       $xmlname->appendChild($xmltrivial);

// //    Synonyms are retrieved from the pubchem/NCI db and added to the XML for searching
//       if (isset($inchi_key[0])) {
//         $url = 'http://cactus.nci.nih.gov/chemical/structure/' . urlencode($inchi_key[0]) . '/names/xml';

//         if (islandora_chem_sp_url_exists('http://cactus.nci.nih.gov')) {
//           $iupac_xml = new DOMDocument();
//           $iupac_xml->load($url);
//           $nodes = $iupac_xml->getElementsByTagName('item');
//           $nodeno = $nodes->length;

//           $synonym = '';

//           for ($i = 0; $i < $nodeno; $i++) {
//             $iupac = $iupac_xml->getElementsByTagName('item')->item($i)->nodeValue;
//             $iupac = htmlentities(ucfirst(strtolower($iupac)));
//             $synonym .= ', ' . $iupac;
//           }

//           $xmlsynonym = $dom->createElement("alternative", ltrim($synonym, ','));
//           $xmlsynonym->setAttribute("id", "s1");
//           $xmlsynonym->setAttribute("type", "synonym");
//           $xmlname->appendChild($xmlsynonym);
//         }
//         else {
//           drupal_set_message(t('Could not connect to @url', array('@url' => $url)), 'warning');
//         }
//       }
//       else {
//         $xmlsynonym = $dom->createElement("alternative", 'None found');
//         $xmlsynonym->setAttribute("id", "s1");
//         $xmlsynonym->setAttribute("type", "synonym");
//         $xmlname->appendChild($xmlsynonym);
//       }

//       $xmlfragment = $dom->createElement("alternative", $fragment);
//       $xmlfragment->setAttribute("id", "f1");
//       $xmlfragment->setAttribute("type", "fragment");
//       $xmlname->appendChild($xmlfragment);

//       $xmlproperty = $dom->createElement("property");
//       $xmlproperty->setAttribute("id", "p1");
//       $xmlproperty->setAttribute("title", "Molecular Weight");
//       $xmlname->appendChild($xmlproperty);

//       $xmlscalar = $dom->createElement("scalar", $mw_value);
//       $xmlscalar->setAttribute("id", "s1");
//       $xmlscalar->setAttribute("dictRef", "nonSi:mw");
//       $xmlproperty->appendChild($xmlscalar);

//       $xmlformula = $dom->createElement("formula");
//       $xmlformula->setAttribute("id", "f1");
//       $xmlformula->setAttribute("concise", $mf_value);
//       $xmlname->appendChild($xmlformula);

//       $xmlfingerprint = $dom->createElement("alternative", implode(', ', $fingerprint));
//       $xmlfingerprint->setAttribute("id", "fp1");
//       $xmlfingerprint->setAttribute("type", "fingerprint");
//       $xmlname->appendChild($xmlfingerprint);

//       $xmlsmiles = $dom->createElement("alternative", $smiles[0]);
//       $xmlsmiles->setAttribute("id", "sm1");
//       $xmlsmiles->setAttribute("type", "smiles");
//       $xmlname->appendChild($xmlsmiles);

//       $xmlidentifier = $dom->createElement("identifier");
//       $xmlname->appendChild($xmlidentifier);

//       $xmlinchi = $dom->createElement("inchi", $inchi_value);
//       $xmlidentifier->appendChild($xmlinchi);

//       foreach ($inchi_key as $key) {
//         $xmlinchikey = $dom->createElement("inchikey", $key);
//         $xmlidentifier->appendChild($xmlinchikey);
//       }

//       $xmlmetadata = $dom->createElement("metadata");
//       $xmlmetadata->setAttribute("id", "m1");
//       $xmlmetadata->setAttribute("name", "dc:identifier");
//       $xmlmetadata->setAttribute("content", $spider_url);
//       $xmlname->appendChild($xmlmetadata);

//       $dom->save($filename);
//     }
//     else {
//       drupal_set_message(t('XML file already exists in the filesytem!'), 'error');
//       $returnValue = FALSE;
//     }

//     if ($returnValue == TRUE) {
//       $_SESSION['fedora_ingest_files']["$dsid"] = $filename;
//       return TRUE;
//     }
//     else {
//       return $returnValue;
//     }
    }
  }
}