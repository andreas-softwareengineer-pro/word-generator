<?php header("Content-type: text/html; encoding=utf-8");
  //error_reporting(E_ALL); //for debugging
  session_start();
  if (!isSet($_SESSION['generated_words']))
  {
	  $_SESSION['generated_words'] = array();
	  $_SESSION['basename'] = array();
	  $_SESSION['produced_no'] = 0;
	  $_SESSION['produced_min'] = 0;
	  $_SESSION['produced_max'] = 0;
	  $_SESSION['settings'] = [];
	  $_SESSION['quantile_precision'] = [];
  }
  $generated_keep = 4;

  if (isSet($_REQUEST['session_close'])) {
		  session_destroy();
		  session_start();
  }

  if (isSet($_REQUEST['var_settings'])) {
	  //Reuse current variant's seetings as a request
	  foreach($_SESSION[$_SESSION['produced_no']] as $name)
	  	if ($name != 'var_settings')
	  		echo $name."<BR>";
	  		$_REQUEST[$name] = $_SESSION[$_SESSION['produced_no']][$name];
	}

$sample_dict = array();
$sample_dict_dir = "sample_dict";
if ($handle = opendir($sample_dict_dir)) {
    while (false !== ($entry = readdir($handle))) {
        if ($entry != "." && $entry != ".." && "lex" == pathinfo($entry,PATHINFO_EXTENSION)) {
        {
	            $n= pathinfo($entry,PATHINFO_FILENAME);
	            $sample_dict[preg_replace('/_/',' ',$n)] = $n;
    	}
    }
    }
    closedir($handle);
}

  //session_set_save_handler(onSessionOpen, onSessionClose, readSession, writeSession, onSessionDestroy, gc);
  if (isSet($_REQUEST['download_words'])) {
	  if (isSet($_SESSION['generated_words'][$_SESSION['produced_no']]))
	  {
		  $filename_hint = preg_replace('/[^.\w]+/u', '_',  $_SESSION['basename'][$_SESSION['produced_no']])."_gen.txt";
		  $filename_hint_encoded = urlencode($filename_hint);
		  
		  header("Content-Disposition: attachment; filename=$filename_hint; filename*=UTF-8''$filename_hint_encoded;");
		  echo(join("\r\n", $_SESSION['generated_words'][$_SESSION['produced_no']]));
		  exit();
	  } else {
		  $errmsg = "<DIV class=error>No words to download. Please generate first!</DIV>";
	  }
  }

    if (isSet($_REQUEST['view_sample'])) {
	  if (isSet($_REQUEST['dict_sample']))
	  {
		  $f = $sample_dict[$_REQUEST['dict_sample']];
		  if (!isSet($f)) exit;
		  header("Content-type: text/html; encoding=utf-8;");
		  echo "<HTML><BODY>";
		  $content = file($sample_dict_dir."/".$f.".lex");
		  foreach ($content as $r) {
			  if (preg_match('/:/',$r)) {
				  echo "<B>$r</B><BR>";}
			  else {echo "$r<BR>";}
	  		}
		  echo "</BODY></HTML>";
	  		exit;
	  } else {
		  $errmsg = "<DIV class=error>No Sample found!</DIV>";
	  }
  }

  if (isSet($session_root)) {$_SESSION['session_root'] = $session_root;}


  if (isSet($_REQUEST['generate']))
  {
  	  //Herein we claim that user WANTS to produce a new variant:
  	  $_SESSION['produced_no'] = $_SESSION['produced_max'];
  	  //but we increment $_SESSION['produced_max'] if only that attempt is successfull

  	  $file_ok = false;
  	  if (!($_REQUEST['dict_side']==="server")) {
		if (isSet($_FILES['dict_file'])) {
			$dict_file = $_FILES['dict_file']['tmp_name'];
			$file_name = $_FILES['dict_file']['name'];
			$file_ok  = ($_FILES['dict_file']['error']===0);
  		}
  	  } else {
	  	  $file_name = $sample_dict[$_REQUEST['dict_sample']];
	  	  $dict_file = $sample_dict_dir."/".$file_name.".lex";
	  	  $file_ok = true; //???
  		}
  	  if ($file_ok) {
  	  $opt = "";
  	  $f0 = floatval ($_REQUEST['abstr_factor']);
  	  $f="";
  	  //$thr = array(0.,.3, .4,.45, .5,.6);
  	  $thr = array(0.,.25, .4,.5, .6,.7);
  	  for ($i=0; $i<6; ++$i) {
	  	  if ($i > 0) {$f .= ",";}
	  	  $thr_val = $f0/100.*floatval ($_REQUEST["eq$i"])/100.;
	  	  $thr_bound = $thr["$i"];
  			$f .= "$thr_bound:$thr_val";
	  }
  	  
  	  $c = intval ($_REQUEST['count']);
  	  $n = intval ($_REQUEST['max_ng']);
  	  $s = intval ($_REQUEST['max_syl']);
  	  $l = escapeshellarg ($dict_file);
  	  $a = floatval ($_REQUEST['subsequent_multiplier'])/100.;
  	  $b = floatval ($_REQUEST['sc'])/100.;

   	  if (isSet($_REQUEST['display_score'])) {$opt .= " -%";}
  	  if (isSet($_REQUEST['sort_score'])) {$opt .= " -T";}
  	  if (isSet($_REQUEST['pick_top']))  {$opt .= " -t";}	  
  	  if (isSet($_REQUEST['allow_hit'])) {$opt .= " -H";}
  	  if (isSet($_REQUEST['mark_hit'])) {$opt .= " -C";}
  	  if (isSet($_REQUEST['verbose'])) {$opt .= " -v 1";}
  	  if (isSet($_REQUEST['evaluate_prec'])) {$opt .= " -p";}
  	  
  	  //unset($_SESSION['generated_words']);
  	    $status = 0;
  	    $exec_file = escapeshellarg (__DIR__."/wg.exe");
  	    exec("$exec_file -f $f -a $a -n $n -b $b -c $c -s $s $opt $l 2>&1",
  			$_SESSION['generated_words'][$_SESSION['produced_no']],$status );
  		$_SESSION['basename'][$_SESSION['produced_no']] = pathinfo($file_name,PATHINFO_FILENAME);
 
  		if ($status != 0) {
	  		$errmsg = "<font color=red>".
	  			"ERROR, STATUS = $status ".join("\n",$_SESSION['generated_words'][$_SESSION['produced_no']]);
	  			"</font>";
	  			$assume_repairable_error=true;
	  	} else {
		  			$produced_keep = 4;
		  			$_SESSION['produced_max'] = $_SESSION['produced_no'] + 1;
		  			if ($_SESSION['produced_max'] - $_SESSION['produced_min'] > $produced_keep) {
			  			 unset($_SESSION['generated_words'][$_SESSION['produced_min']]);
		  				++$_SESSION['produced_min'];
		  				}
	  				}
	  	$_SESSION['settings'][$_SESSION['produced_no']]=[];
	  	foreach ($_REQUEST as $p=> $v)
	  		if ($p!='generate')
  			{
	  			$_SESSION['settings'][$_SESSION['produced_no']][$p] = $v;
  			}
	    } else {
		    $errmsg="<DIV class=error>Please select a dictionary file to generate from</DIV>";
		    $assume_repairable_error=true;
	    }
	     }
	   elseif (isSet($_REQUEST['prev_variant'])) {
	    if ($_SESSION['produced_no'] > $_SESSION['produced_min']) {
		    --$_SESSION['produced_no'];
	    }
	    if ($_SESSION['produced_no'] <= $_SESSION['produced_min'])
		if ($_SESSION['produced_max'] > $_SESSION['produced_min']) {
		if ($_SESSION['produced_min'] > 0) {
			  $errmsg = "This is the least recent variant kept.";
		} else {
		  $errmsg = "This is the first available variant.";
		}} else {
		  $errmsg = "No variant was generated yet.";
		}
	  } elseif (isSet($_REQUEST['next_variant'])) {
	    if ($_SESSION['produced_no'] < $_SESSION['produced_max'] - 1) {
			  ++$_SESSION['produced_no'];
		} else {
			  if ($_SESSION['produced_max'] > $_SESSION['produced_min']) {
				  $_SESSION['produced_no'] = $_SESSION['produced_max'] - 1;
				  $errmsg = "This is the last variant generated.";
			  } else {
				  $errmsg = "No variant was generated yet.";
			  }				  
		}
	  }

function reconcile_req_val($name, $allowed_values) {
	if ( isSet($_REQUEST[$name]) ) {
		$dist =100000000.; $v = $_REQUEST[$name];
		foreach ($allowed_values as $av) {
			if (abs($v - $av) < $dist) {
				$_REQUEST[$name] = $av;
				$dist = abs($v - $av);
			}
		}
	}
}

//A convenience function that outputs html code for a check/radio button
//It uses http request values, if any, to set the initial button checking
function html_input($type, $name, $value, $defval, $params, $label=null) {
    echo "\t<SPAN style=\"display: inline-block;\"><INPUT TYPE=".
    $type." ID=\"".$name."\" NAME=\"".$name."\" VALUE=\"".$value."\" ".
    (((@$_REQUEST[$name] ?: $defval) == $value)? 'checked ' : '').
    $params."/>\n";
    if (!isSet($label)) $label = $value;
    echo "<LABEL for=\"$name\">$label</LABEL></SPAN>";
}
?>

<?php if ($_SERVER['REQUEST_METHOD']!='POST' || isSet($_REQUEST['var_settings']))  { ?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
  <html><head>
  <link rel="stylesheet" href="//code.jquery.com/ui/1.11.4/themes/smoothness/jquery-ui.css">
  


  <script src="//code.jquery.com/jquery-1.10.2.js"></script>
  <script src="//code.jquery.com/ui/1.11.4/jquery-ui.js"></script> 
  <link rel="stylesheet" href="/resources/demos/style.css">

  <style>

  #eq div > span {
    height:70px; width:7px; float:left; margin:12px; margin-right: 6pt;
  }
  #eq_wildplus div > span {
    height:60px; width:7px; float:left; margin:12px;  margin-top:22px;
  }
  #eq_wildplus {
	margin-left: 0.7em;
  }
  #eq_master {
	margin-left: 20px;
  }
  #eq  td > div {
  }
  #eq div input,#eq_wildplus div input {
	  display: inline-block;
	  overflow: hidden;
  }
  #eq_sc div > span {
	height:7px; width:80%; display:inline-block; margin:12px;
  }
  
  #eqdiv5 .ui-slider-range,#eqdiv4 .ui-slider-range {background: green;}
  #eqdiv3 .ui-slider-range,#eqdiv2 .ui-slider-range {background: yellow;}
  #eqdiv1 .ui-slider-range,#eqdiv0 .ui-slider-range {background: red;}
  #eq_wildplus .ui-slider-range {background: #c00060;}
  #eq_master .ui-slider-range {background: #0000a0;}
  #eq_sc .ui-slider-range {background: #ffc080;}
  #eq input,#eq_wildplus input,#eq_sc input {
		width: 20px; border: 0px; font-size:80%;
		background: transparent;
		float:left;
	}
  #eq_master .ui-slider .ui-slider-handle  {width:10px; border-color:#7070ff; background:#b0b0ff; padding-left: 5px;
    height: 13px;} 
  .ui-slider .ui-slider-handle  {width:10px; padding-left: 5px;
    height: 13px;} 
  

  </style>

  <style>
  </style>

  <script>
  function onmax_ng() {	  
  	if ($('input[name=max_ng]:checked', '#wg_form'). val() > 3) {
		$('#wildplus_label, #eq_wildplus').show('slow');
  	} else {
		$('#wildplus_label, #eq_wildplus').hide('slow');
  	}
  }
  </script>
  <script>
  function onallow_hit() {
  	if ($('#allow_hit').is(":checked")) {
		$('#mark_hit_span').show('slow');
  	} else {
		$('#mark_hit_span').hide('slow');
  	}
  }
  </script>
  <script>
  function ondict_side() {
  	if ($('#dict_side').is(":checked")) {
		$('#dict_server').show('slow');
		$('#dict_client').hide('slow');
  	} else {
		$('#dict_server').hide('slow');
		$('#dict_client').show('slow');
  	}
  }
  </script>
  <script>

  function oneeq() {
     $( "#eq1 > span,#eq2 > span,#eq3 > span,#eq4 > span,#eq5 > span,#eq0 > span" )
    	.each(function() {

      // read initial values from markup and remove that

      //var value = parseInt( $( this ).text(), 10 );
      //var input = $(this).parent().find('> input');
      $(this).slider('value',100);
})
}

function reseteq() {
     $( "#eq1 > span,#eq2 > span,#eq3 > span,#eq4 > span,#eq5 > span,#eq0 > span" )
    	.each(function() {
      $(this).slider('value',$(this).slider("option", "def_value"));
})
}

  function fs(val) {return val>=100? "65%": "70%";}
  $(function() {

    // setup graphic EQ

    $( "#eq div > span,#eq_wildplus div > span,#eq_sc div > span" ).each(function() {

      // read initial values from markup and remove that

      var value = parseInt( $( this ).text(), 10 );
      var input = $(this).parent().find('> input');
      input.val(value);
      input.css("font-size",  fs(value));

	  var ori = input[0].getAttribute("orientation");
	  var minval = input[0].getAttribute("min");
	  minval = (minval == null) ? 0 : parseInt( minval );
	  var maxval = input[0].getAttribute("max");
	  maxval = (maxval == null)? 100 : parseInt( maxval );

      $(this).empty().slider({

        value: value,
        def_value: value, //my add-on

        range: "min",

        min: minval,
        
        max: maxval,

        animate: true,

        orientation: (ori ? ori : "vertical"),
        
            change: function() {
        var value = $( this ).slider("option","value");
        input.val(value);
      input.css("font-size",  fs(value));
    },
    slide: function() {
        var value = $(this).slider("option","value");
        input.val(value);
      input.css("font-size",  fs(value));
    }

        

      });
      //$( this ).find ('.ui-slider-range').css('background', 'rgb(255,0,0)');

    });
  });

  </script>


<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<link type="text/css" rel="stylesheet" href="wg.css" media="all" /> 
</head><body onresize="onbodyresize();" onready="onbodyresize();">
<TABLE width="100%"><TR><TD><font size=+2>Word generator</font><I>(v1.2) </I></TD><TD class="hdr_info"><font style="font-family: tahoma;" size=-1> <A HREF="wg-updates.html">Info & updates</A>
	</font></TD></TR></TABLE>
<HR>
  <DIV  id="wait" style="display:none" ><IMAGE height=50 src=/wait.gif></DIV>
  <?php echo ($errmsg); ?>

  <FORM METHOD=POST id=wg_form TARGET="word_f" ACTION="wg.php" enctype="multipart/form-data">
  <TABLE   width="100%" cols=3 class=option_table>
    <TR><TD>
	</TD>
  <TD rowspan=4>

 <DIV style="display:inline-block; float:left;">
      	<A HREF="#abstr_factor_hint" class="hint_a" style="align:center;"><h4>Abstraction factors</h4></A>
 <A HREF="#superconcrete_hint" class="hint_a">Required confidence, %</A>
<div id="eq_sc">
  <div>
  <input name="sc" type=text min=10 max=90 orientation="horizontal"/>
  <span><?php echo @$_REQUEST["sc"]?:70; ?></span>
  </div>
</div>
 </DIV>
  <TABLE style="display:inline-block; float:left; margin-left:2%;margin-right:2%;"><TR><TD>
<SPAN id="wildplus_label" style="font-size:90%;"><A HREF="#subsequent_multiplier_hint" class="hint_a">wildcard+</A></SPAN>
</TD></TR><TR><TD>
  <div id="eq_wildplus">
  <div>
  <span><?php echo @$_REQUEST["subsequent_multiplier"]?:70; ?></span>
  <input name="subsequent_multiplier" type=text  style="margin-left: -10px; width=20pt; float=top;">
  </div>
  </div>
</TD></TR> </TABLE>
<table id="eq" style="display:inline-block; float:left;>
<TR style="margin-bottom:4px; font-size:80%"><TD colspan=6>
  <span class="ui-icon ui-icon-signal" style="float:left;"></span>
  <A class="hint_a"  HREF="#confidence_multiplier_hint">Confidence</A> dependent multipliers, %&nbsp;
  <A class="control_a" HREF="javascript:oneeq();">100%</A>
  <A class="control_a" HREF="javascript:reseteq();">Reset</A>
</TD><TD><B><div align=center>master</div></B>
</TD></TR><TR>
<?php 
$def_confidence_factors = [0,10,40,70,100,100];
for ($i=0; $i<6; ++$i)
echo "<TD>
  <div id=\"eqdiv$i\">
  <span>".(@$_REQUEST["eq".$i]?:$def_confidence_factors[$i]).'</span>
  <input name="eq'.$i.'" type=text  style="margin-bottom: -10px; width=20pt; float=left;">
  </div>
</TD>'; ?>
<TD>
  <div id="eq_master">
  <span><?php echo @$_REQUEST["abstr_factor"]?:90; ?></span>
  <input name="abstr_factor" max=200 type=text  style="margin-left: -10px; width=20pt; float=top;">
  </div>
</TD></TR>
</table>

<TD class="rack_handle" rowspan="4"><A target=_blank HREF="wg_note.pdf" ><img src="handle.png" TITLE="How does it work?" ALT="How does it work?"></HREF>
</TD></TR>
<TR><TD></TD>
</TR><TR>
<TD>
</TD></TR><TR><TD>
</TD></TR>
</TABLE> 
  <TABLE width=100% cols=5 style="margin-top:8px;" class=option_table>
  <TR><TD>  <A href="#max_ng_hint" class="hint_a"> N-Gram size</A></TD>
  <TD colspan=3>Generated Word Count (max.<INPUT TYPE="number" name="max_syl" value=<?php   echo $_REQUEST['max_syl']?:10; ?> min=1 style="width:3em;"/>&nbsp;<A href="#syl_hint" class="hint_a">syllables</A>
)
  </TD>
  </TD><TD class="rack_handle" rowspan="6"><A target=_blank HREF="wg_note.pdf"><img src="handle.png" TITLE="How does it work?" ALT="How does it work?"></HREF>
  </TR>
  <TR><TD>
  <?php
  $default_max_ng = "5";
  for ($max_ng = 3; $max_ng<6; $max_ng++)
  {
	html_input("RADIO", "max_ng", $max_ng, $default_max_ng,
		'onchange="onmax_ng();"');
  }
  ?>  </TD><TD colspan=3>
  <?php $count_opts = 
	["200", "500", "1000", "2000", "5000"];
	reconcile_req_val("count", $count_opts);
	foreach ($count_opts as $val)
	{
  		html_input("RADIO", "count", $val, "500", '');
	}
  ?> 
  </TD></TR>
<!-------  <TR><TD>
  Pick
------->  </TD><TD>
  <A href="#dict_word_hint" class="hint_a">Dictionary words</A>
  </TD><TD>
    <A href="#score_hint" class="hint_a">Probability Score</A>
  </TD>
  <TD>
    <A HREF="#evaluate_prec_hint" class="hint_a">Precision</A>
  </TD></TR>
<!-------  <TR><TD>
  <?php html_input("RADIO", "pick", "top", "", "","Top"); ?>
<INPUT TYPE="RADIO" NAME="pick" VALUE="200" checked/>Random
------->  </TD><TD>
<?php html_input("CHECKBOX", "allow_hit", "1", "0", 'onchange="onallow_hit();"', "Allow"); ?>
  <SPAN id="mark_hit_span" style="flow:left;">
  <?php html_input("CHECKBOX", "mark_hit", "1", "0", '', 'Mark(#)' ); ?></span>
  </TD><TD>
  <?php html_input("CHECKBOX", "display_score", "1", "0", "", 'Display'); ?>
  <?php html_input("CHECKBOX", "sort_score", "1", "0", "", 'Sort by'); ?>
  <?php html_input("CHECKBOX", "pick_top", "1", "0", "", 'Pick top'); ?>
  </TD><TD>
  <?php	html_input("checkbox", "evaluate_prec", 1, 0, "", 'evaluate');?>
  </TD>
 </TR><TR>
  <TD style="padding-top:8px;" colspan=3>
  <A href="#source_dict_hint" class="hint_a">Dictionary file</A>
  <SPAN id=dict_client style="padding-left:5pt;"><INPUT TYPE="FILE" NAME="dict_file" ID="dict_file"/>
  </SPAN>&nbsp;
  <?php html_input("CHECKBOX", "dict_side", "server", "", 'onchange="ondict_side();"', 'sample');?>
  <SPAN id=dict_server>:&nbsp;<INPUT TYPE=SUBMIT VALUE="View" class=small_btn NAME=view_sample><select NAME="dict_sample" ID="dict_sample">
  <?php
  foreach (array_keys($sample_dict) as $name) {
	  echo "<option value=\"$name\"";
	  if (@$_REQUEST["dict_sample"] === $name)
	  	echo " selected";
	  echo ">$name</option>";
  } ?>
  </select></SPAN> 
  </TD>
  </TR>
  
  </TABLE>
<DIV class=hint id=abstr_factor_hint>
		<h3>Abstraction Factor</h3>
		<h4>Every generated word is produced according to the probability score of prefix continuation.
		 To determine this score it considers not just concrete prefixes found in the source dictionary but also abstracted ones
		 (where one or more phonemes are substituted with abstracted symbols like the consonant and the vowel.)
		 However, abstracted patterns yield lower scores than concrete ones, in order to make concrete prefixes preferable.
		 The Abstraction Factor defines a multiplicator applied to the score when we turn one phoneme position into an abstracted symbol.
		 The higher the abstraction factor is, the richer variety of words we obtain but their phonotactics may become less similar to that of the original dictionary.
		<BR><div align=center><A HREF="#" class=hint_a>Close</A></DIV></h4></DIV> 
<DIV class="cover" onclick="window.location.href='#';"></DIV>
<DIV class=hint id=subsequent_multiplier_hint>
		<h3>Subsequent phoneme multiplier ("wildcard +")</h3>
		<h4>Each time we increase the abstracted phonemes count in our probability calculations we apply this additional multiplier to the <A HREF="#abstr_factor_hint">Abstraction Factor.</A><BR>
		Example: let (master*confidence_based) abstraction factor be 0.5 and Subsequent phoneme multiplier be 0.7 .<BR>
		Then, {'a','b','c','d'} 4-gram in generated word sheel will be considered the same probable as {'a','b','c','d'} one in the source dictionary,
		 plus 0.5 times as probable as {'a',&lt;consonant&gt;,'c','d'} and  {'a',&lt;vowel&gt;'c','d'}
		 plus (0.5*0.7=0.35) times as probable as &lt;vowel&gt;,&lt;consonant&gt;,'c','d'}.
		<BR>See also <A HREF="#confidence_multiplier_hint">Confidence dependent multiplier</A>
		<BR><div align=center><A HREF="#" class=hint_a>Close</A></DIV></h4></DIV> 
<DIV class="cover" onclick="window.location.href='#';"></DIV>
<DIV class=hint id=superconcrete_hint>
		<h3>On-the-fly pruning - Minimum required confidence</h3>
		<h4>To enforce more proper phoneme combination choice in a case of sparse training dictionary, an additional pruning of low-confidence N-Grams is applied.
		This means rejecting a word-in-progress once some produced N-Gram doesn't meet a minimum confidence threshold.
		Please note it's not the same as avoiding lower confidence NGrams at each step. In contrast, the former method doesn't force completing of 
		every 'weird' word prodction process. That approach
		enables significant increase of overall word generation precision. But high confidence thresholds should be used with care as
		most 'high-hanging fruits' are unlikely to be hit with them.
		<BR><div align=center><A HREF="#" class=hint_a>Close</A></DIV></h4></DIV> 
<DIV class="cover" onclick="window.location.href='#';"></DIV>
<DIV class=hint id=confidence_multiplier_hint>
		<h3>Confidence dependent multiplier</h3>
		<h4>When we apply an abstracted N-gram from the source dictionary to generate words,
		we first evaluate the confidence of abstraction. If we, say, have seen multiple different consonants
		occupying the same posintion in a N-gram, we treate an abstraction of them to the Consonant class as confident. Otherwise, if we have just seen just consonants at this position, we treate the abstraction as of low confidence (in another word, we don't expect an arbitrary consonants to be suitable at that position.)
		You may select an additional multiplier dependent on the confidence level to be applied over the <A HREF=#abstr_factor_hint>Abstraction factor</A>. You may, say, force more confident abstractions usage in generated words (the default setting) or, if you prefer, to encourage unusual words occurence
		by increasing the multiplier for lower confidence levels.
		<BR><div align=center><A HREF="#" class=hint_a>Close</A></DIV></h4></DIV> 
<DIV class="cover" onclick="window.location.href='#';"></DIV>
<DIV class=hint id=syl_hint>
		<h3>(Quasi) Syllable limit</h3>
		<h4>We use a simplified syllable count computation for this limit. Any cluster of one or more vowels is accounted as shaping one syllable. For example, "Austria" is a two quasi-syllable word.
		<BR>
		<BR><div align=center><A HREF="#" class=hint_a>Close</A></DIV></h4></DIV> 
<DIV class="cover" onclick="window.location.href='#';"></DIV>
<DIV class=hint id=max_ng_hint>
		<h3>N-Gram size</h3>
		<h4>We generate words by reproducing the probabilistic distribution of phoneme sequences found in the original dictionary.
		 The <STRONG>N-Gram size</STRONG> option defines the maximum lengths of phoneme sequences considered (named N-Grams, where N is the length.)
		 At too small N we may miss important phonotactic patterns while too high N leads to little difference between generated and original words as well as to slow computation.
		 Usually a value of 4-5 is the most reasonable.
<BR><div align=center><A HREF="#" class=hint_a>Close</A></DIV></h4></DIV> 
<DIV class="cover" onclick="window.location.href='#';"></DIV>
<DIV class=hint id=dict_word_hint>
		<h3>Dictionary Words</h3>
		<h4>You may allow the appearance of generated words that exactly match words found in the original dictionary. By default, such words are filtered out.<BR>
		Thus, if both "Allow" and "Mark(#)" button checked, such words will be marked with a trailing hash character. 
<BR><div align=center><A HREF="#" class=hint_a>Close</A></DIV></h4></DIV> 
<DIV class=hint id=evaluate_prec_hint>
		<h3>Precision evaluation</h3>
		<h4>This mode displays estimated precision of training dictionary lexeme 'hitting'.
		Using that feature makes playing with Word Generator options and parameters much more meaningful.
		While evaluating a given word 'hitting' probability, it assumes that this particular word is excluded from the dictionary.
		Results are grouped by <I>quantiles</I> of most 'challenging' words. Say, <font color="#604020">0.05</font>, <font color="#604020">0.25</font>, <font color="#604020">1</font> quantile precisions displayed in a result table are computed over
		<font color="#604020">5%</font>, <font color="#604020">25%</font> and <font color="#604020">100%</font> 'highest-hanging fruits', respectively. (The last one is (also) the mean precision over the whole dictionary)
		<BR><div align=center><A HREF="#" class=hint_a>Close</A></DIV></h4></DIV> 
<DIV class="cover" onclick="window.location.href='#';"></DIV>
<DIV class="cover" onclick="window.location.href='#';"></DIV>
<DIV class=hint id=score_hint>
		<h3>Probability Score</h3>
		<h4>The Probability Score is an internal metric of a word appearance likelihood. (In the current implementation it's not necessary equal to the probability itself.)
		You may use the following controls to deal with the Score:
		<UL>
		<LI><Strong>Pick top</Strong> - run a flow that hunts for top scored words rather than making random 'shots'. When using it, please note that the word collection will not tend to show much variety in the length as well as in overall flavor.</LI> 
		<LI><Strong>Sort by</Strong> - sort the output by word Scores</LI>
		<LI><Strong>Display</Strong> - display the Score next to every generated word like <I>word (0.0001)</I></LI>
		</UL>
	<div align=center><A HREF="#" class=hint_a>Close</A></DIV></h4></DIV> 
<DIV class="cover" onclick="window.location.href='#';"></DIV>
<DIV class=hint id=source_dict_hint>
		<h3>Source Dictionary</h3>
		<h4>The dictionary used to learn information on phonotactics of the language which we are about to generate words in.<BR>
		This should be a plain text file (in Ascii or UTF8 encoding) that contain:
		<UL>
		<LI>A <strong>Vowels</STRONG>line containing a comma-separated enumeration of all vowel phonemes available. It's ok for a phoneme to be represented by a combination of characters.<BR>
		<I>Example:</I> Vowels: a, aa, e, ee, i, ii, o, oo, u, uu if </LI> 
		<LI>A similar line for <Strong>Consonants</Strong></LI>
		<LI>A plain list of words, one per line</LI>
		</UL>
<BR><div align=center><A HREF="#" class=hint_a>Close</A></DIV></h4></DIV> 
<DIV class="cover" onclick="window.location.href='#';"></DIV>

     <INPUT TYPE="Submit" NAME="prev_variant" ID="prev_variant" VALUE="<" ALT="Previous generated variant"/>
     <INPUT TYPE="Submit" NAME="next_variant" ID="next_variant" VALUE=">" ALT="Next generated variant"/>
     <INPUT TYPE="Submit" NAME="generate" ID="generate" VALUE="Generate" />
     <INPUT TYPE="Submit" NAME="download_words" VALUE="Download"/>
     <INPUT TYPE="Submit" NAME="session_close" VALUE="Close session"/>
	 <SPAN style="float:right">
	 <?php html_input("checkbox","verbose",1,0,"style=anchor:right;", 'verbose');?>
     </SPAN>     
  </FORM>
<iframe name="word_f" onload="$('#wait').hide('slow');" id="word_f" style="display:block; height:400px; width:100%; overflow:auto; ">
  </IFRAME><BR>
  <DIV class="footer-note">
  Word generator does not store any data that you upload.
  <I><HR></DIV>
  <DIV CLASS="footer">Word generator is written by <A HREF="http://www.andreas.ru" class="person_a"> Andreas Scherbakov</A> as part of a project run by <A HREF="http://www.nthieberger.net" class="person_a">Nick Thieberger</A> with funding from <A HREF="http://www.arc.gov.au" class="person_a">ARC</A> Future Fellowship (FT140100214)
  </I>

<!--------   <DIV align=center><font size=-2>Author & Support: Andrey S Shcherbakov<A HREF="http://www.softwareengineer.pro/"> http://www.softwareengineer.pro/</A><BR>SINQ Science & Production Co.,Ltd.</font></DIV>
--------->
<script>
var $submitActor = null;
var $submitActors = $('form').find( 'input[type=submit]' );

$('form').submit(function(){
    if (submitActor.name==='generate' || submitActor===$('#prev_variant') || submitActor===$('#next_variant'))
    {
	    $('#wait').show('slow');
    }
    return true;
});
$('iframe').ready(function(){
    $('#wait').hide('slow');
    return true;
});
$submitActors.click( function( event )
    {
      submitActor = this;
    });

function onbodyresize() {
	$( ".rack_handle" ).each(function() {
		if ($( "body" ).width() >= 750)
			$(this).show();
		else
			$(this).hide();
	});
}

onbodyresize();
onallow_hit();
ondict_side();

</script>

</BODY></HTML>
<?php exit();
}

function horizAssocTableRender($assoc, $variant, $from_name, $to_name, $hdr) {
		$colspan = sizeof($assoc[$variant]);
		$rc = "\n<TABLE class=\"result\" width=\"100%\">\n\t<TR class=\"param_header\">".
		"<TD rowspan=2 class=\"info_header\">Attempt</TD><TD colspan=$colspan>".$hdr.
		"</TD></TR>\n\t<TR class=\"param_name\">";
		foreach (array_keys($assoc[$variant]) as $q)
		 	$rc.="<TD>$q</TD>";
		$rc.="</TR>\n";

		//Find the highest values in each nomination
		$highest = array();
		foreach (array_keys($assoc[$variant]) as $q) {
			for ($other_var = $_SESSION['produced_min'];
				$other_var < $_SESSION['produced_max']; $other_var++)
				if (isSet($assoc[$other_var][$q]) && (
					!isSet($highest[$q]) || $highest[$q] < $assoc[$other_var][$q]))
					$highest[$q] = $assoc[$other_var][$q];
		}
		
		//Show the table
		for ($other_var = $_SESSION['produced_max']-1;
			$other_var >= $_SESSION['produced_min']; $other_var--)
		{
			$row_class = $variant == $other_var ? "this_result" : "other_result";
			$rc.="\t<TR class=\"$row_class\"><TD class=\"param_header\">$other_var";
			if ($other_var == $variant)
				$rc.="<font color=\"green\" size=\"+1\">&nbsp;&#x2713;</font>";
			$rc.="</TD>";
			foreach (array_keys($assoc[$variant]) as $q )
			{
				$rc.="<TD>";
				$v = $assoc[$other_var][$q];
				if ($highest[$q] <= $v) $rc.="<B>";
				$rc .= sprintf("%.3g", $v);
				if ($highest[$q] <= $v) $rc.="</B>";
				$rc.="</TD>";
			}
			$rc.="\t</TR>\n";
		}
		$rc.="</TABLE>\n;";
		return $rc;
}

function specialInfoRender($info, $variant) {
	if (preg_match_all('/\s*Quantile precision:\s+(.*\S)\s*/', $info, $prec_info)) {
		$assoc = array_reverse(json_decode($prec_info[1][0], true), true);
		$_SESSION['quantile_precision'][$variant] = $assoc;
		return horizAssocTableRender(
			$_SESSION['quantile_precision'],
			$variant,
			"Quantile",
			"Precision",
			"Estimated <A href=\"precision.html\">Precision</A> at 'high-hanging fruit' quantile</TD>"
);
	}
	return "";
}

?>
<HTML><HEAD>
<link type="text/css" rel="stylesheet" href="wg.css" media="all" /> 
</HEAD><BODY class="output">
<?php echo $errmsg." ";
$url=strtok($_SERVER["REQUEST_URI"],'?');
if ($_SESSION['produced_no'] < $_SESSION['produced_max']) //A successful production
	echo "<A HREF=\"$url?".http_build_query($_SESSION['settings'][$_SESSION['produced_no']])."\" target=\"__parent__\">Direct link to this variant settings</A><BR>";
  
  $coln = 3;
  $j = $coln;
  $v = $_SESSION['produced_no'];
  $preface = [];
  $gwc = count($_SESSION['generated_words'][$v]);
  for ($i = 0; $i < $gwc; ++$i)
  {
	    if (preg_match("/^Warning:/", $_SESSION['generated_words'][$v][$i])) {
		     $preface[$i]="</TR><TR class=\"warnings\" style=\"background-color: #ffff00;\"><TD style=\"border-width: 2pt;\" colspan=$coln><span class=\"warnings\" style=\"background-color: #ffff00;\">".$_SESSION['generated_words'][$v][$i]."</SPAN></TD></TR><TR>";
		     }
	    else if (preg_match("/^Error:/", $_SESSION['generated_words'][$v][$i])) {
		     $preface[$i]="</TR><TR class=\"errors\" style=\"background-color: #ffd0d0;\"><TD style=\"border-width: 2pt;\" colspan=$coln><span class=\"warnings\" style=\"background-color: #ffff00;\">".$_SESSION['generated_words'][$v][$i]."</SPAN></TD></TR><TR>";
		     }
	    else if (preg_match_all('/^Info:\s+(.*\S)\s*/', $_SESSION['generated_words'][$v][$i], $info)) {
		     $s = specialInfoRender($info[1][0],$v);
		     $preface[$i]= "</TR><TR class=\"infos\"><TD colspan=$coln>".
		     	($s?:$_SESSION['generated_words'][$v][$i]).
		     	"</TD></TR><TR>";
		     }
  }

  foreach ($preface as $s)
		echo $s."<BR>";

  echo "<TABLE>";
  for ($i=0; $i < $gwc; $j+=$coln)
  {
	echo "<TR>";
	    for (; $i < $j; ++$i) if (!isSet($preface[$i]))
    {
    	if (preg_match("/#/", $_SESSION['generated_words'][$v][$i])) {
		     echo "<TD class=\"dict_hit\">".$_SESSION['generated_words'][$v][$i]."</TD>";}
		     else {
		     echo "<TD>".$_SESSION['generated_words'][$v][$i]."</TD>";}
	}
	echo "</TR>";
  }
  echo "</TABLE>";
 ?>
</BODY></HTML>
