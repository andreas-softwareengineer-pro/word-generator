<?php header("Content-type: text/html; encoding=utf-8");
  //error_reporting(E_ALL); //for debugging
  session_start();
  if (!isSet($_SESSION['generated_words']))
  {
	  $_SESSION['generated_words'] = array();
	  $_SESSION['basename'] = array();
	  $_SESSION['produced_no'] = 0;
	  $_SESSION['produced_min'] = 0;
	  $_SESSION['produced_max'] = 0;
  }
  $generated_keep = 4;

  if (isSet($_REQUEST['session_close'])) {
		  session_destroy();
		  session_start();
  }
  
$sample_dict = array();
$sample_dict_dir = "sample_dict";
if ($handle = opendir($sample_dict_dir)) {
	$i = 0;
    while (false !== ($entry = readdir($handle))) {
        if ($entry != "." && $entry != ".." && "lex" == pathinfo($entry,PATHINFO_EXTENSION)) {
            $sample_dict[$i++] = pathinfo($entry,PATHINFO_FILENAME);
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

//  foreach ($_SESSION as $x => $y) {echo "$x=$y";} echo "EOS";
  
//echo "File: " ;echo ($_FILES['dict_file']['tmp_name']); echo  "<BR>";


  if (isSet($_REQUEST['generate'])) {
	  if (isSet($_FILES['dict_file'])) {
	  	//foreach ($_FILES['dict_file'] as $f => $v)
	  	//{ echo ("$f = $v");
  	//}
//	  	rename($_FILES['dict_file']['tmp_name'],$_SESSION['session_root']."/dict.lex");
  	  $file_ok = false;
  	  if (!($_REQUEST['dict_side']==="server")) {
	  	  $dict_file = $_FILES['dict_file']['tmp_name'];
	  	  $file_name = $_FILES['dict_file']['name'];
	  	  $file_ok = ($_FILES['dict_file']['error']===0);
  	  } else {
	  	  $file_name = $sample_dict[$_REQUEST['dict_sample']];
	  	  $dict_file = $sample_dict_dir."/".$file_name.".lex";
	  	  $file_ok = true; //???
  		}
  	  if ($file_ok) {
  	  $opt = "";
  	  $f0 = floatval ($_REQUEST['abstr_factor']);
  	  $f1 = floatval ($_REQUEST['abstr_factor1']);
  	  $f="";
  	  //$thr = array(0.,.3, .4,.45, .5,.6);
  	  $thr = array(0.,.25, .4,.5, .6,.7);
  	  for ($i=0; $i<6; ++$i) {
	  	  if ($i > 0) {$f .= ",";}
	  	  $thr_val = $f0*$_REQUEST["eq$i"]/100;
	  	  $thr_bound = $thr["$i"];
  			$f .= "$thr_bound:$thr_val";
	  }
  	  
  	  $c = intval ($_REQUEST['count']);
  	  $n = intval ($_REQUEST['max_ng']);
  	  $s = intval ($_REQUEST['max_syl']);
  	  $l = escapeshellarg ($dict_file);
  	  $a = floatval ($_REQUEST['subsequent_multiplier']);
  	  $m = floatval ($_REQUEST['min_score']);

   	  if (isSet($_REQUEST['display_score'])) {$opt .= " -%";}
  	  if (isSet($_REQUEST['sort_score'])) {$opt .= " -T";}
  	  if (isSet($_REQUEST['pick_top']))  {$opt .= " -t";}	  
  	  if (isSet($_REQUEST['allow_hit'])) {$opt .= " -H";}
  	  if (isSet($_REQUEST['mark_hit'])) {$opt .= " -C";}
  	  
  	  //unset($_SESSION['generated_words']);
  	  $lastLine =
  	    $_SESSION['produced_no'] = $_SESSION['produced_max'];
  	    $status = 0;
  	    $exec_file = escapeshellarg (__DIR__."/wg.exe");
  	    exec("$exec_file -f $f -a $a -n $n -c $c -m $m -s $s $opt $l 2>&1",
  			$_SESSION['generated_words'][$_SESSION['produced_no']],$status );
  		$_SESSION['basename'][$_SESSION['produced_no']] = pathinfo($file_name,PATHINFO_FILENAME);
 
  		if ($status != 0) {
	  		$errmsg = "<font color=red>".
	  			"ERROR, STATUS = $status ".join("\n",$_SESSION['generated_words'][$_SESSION['produced_no']]);
	  			"</font>";} else {
		  			$produced_keep = 4;
		  			$_SESSION['produced_max'] = $_SESSION['produced_no'] + 1;
		  			if ($_SESSION['produced_max'] - $_SESSION['produced_min'] > $produced_keep) {
			  			 unset($_SESSION['generated_words'][$_SESSION['produced_min']]);
		  				++$_SESSION['produced_min'];
		  				}
	  				} 
	  			}
	    } else {
		    $errmsg="<DIV class=error>Please select a dictionary file to generate from</DIV>";
	    }
	     }
	   elseif (isSet($_REQUEST['prev_variant'])) {
	    if ($_SESSION['produced_no'] > $_SESSION['produced_min']) {
		    --$_SESSION['produced_no'];
	    }
	    if ($_SESSION['produced_no'] <= $_SESSION['produced_min']) {
		if ($_SESSION['produced_max'] > $_SESSION['produced_min']) {
		if ($_SESSION['produced_min'] > 0) {
			  $errmsg = "This is the least recent variant kept.";
		} else {
		  $errmsg = "This is the first variant generated.";
		}} else {
		  $errmsg = "No variant generated.";
		}
	  }} elseif (isSet($_REQUEST['next_variant'])) {
	    if ($_SESSION['produced_no'] < $_SESSION['produced_max'] - 1) {
			  ++$_SESSION['produced_no'];
		}
	    if ($_SESSION['produced_no'] >= $_SESSION['produced_max'] - 1) {
			  if ($_SESSION['produced_max'] > $_SESSION['produced_min']) {
				  $errmsg = "This is the last variant generated.";
			  } else {
				  $errmsg = "No variant generated.";
			  }				  
		}
	  }
?>

<?php if (!isSet($_REQUEST['abstr_factor'])) { ?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
  <html><head>
  <link rel="stylesheet" href="//code.jquery.com/ui/1.11.4/themes/smoothness/jquery-ui.css">
  


  <script src="//code.jquery.com/jquery-1.10.2.js"></script>
  <script src="//code.jquery.com/ui/1.11.4/jquery-ui.js"></script> 
  <link rel="stylesheet" href="/resources/demos/style.css">

  <style>

  #eq > div > div > span {

    height:65px; width: 7px; float:left; margin:12px

  }
  #eq > div > div > input {
    background: transparent;
    float:left;
  }
  #eq_green .ui-slider-range {background: green;}
  #eq_yellow .ui-slider-range {background: yellow;}
  #eq_red .ui-slider-range {background: red;}
  #eq input {width: 20px; border: 0px; font-size:70%;}
  .ui-slider .ui-slider-handle  {width:10px; padding-left: 5px;
    height: 13px;} 
  

  </style>

  <style>
  </style>

  <script>
  function onmax_ng() {	  
  	if ($('input[name=max_ng]:checked', '#wg_form'). val() > 3) {
		$('#subsequent_multiplier_label, #subsequent_multiplier').show('slow');
  	} else {
		$('#subsequent_multiplier_label, #subsequent_multiplier').hide('slow');
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
    $( "#eq > div > div > span" ).each(function() {

      // read initial values from markup and remove that

      var value = parseInt( $( this ).text(), 10 );
      var input = $(this).parent().find('> input');
      $(this).slider('value',100);
})
}  function reseteq() {
    $( "#eq > div > div > span" ).each(function() {
      $(this).slider('value',$(this).slider("option", "def_value"));
})
}

  function fs(val) {return val>=100? "65%": "70%";}
  $(function() {

    // setup master volume

    $( "#master, #master2" ).slider({

      value: 60,

      orientation: "horizontal",

      range: "min",

      animate: true

    });

    // setup graphic EQ

    $( "#eq > div > div > span" ).each(function() {

      // read initial values from markup and remove that

      var value = parseInt( $( this ).text(), 10 );
      var input = $(this).parent().find('> input');
      input.val(value);
      input.css("font-size",  fs(value));


      $( this ).empty().slider({

        value: value,
        def_value: value, //my add-on

        range: "min",

        animate: true,

        orientation: "vertical",
        
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




<link type="text/css" rel="stylesheet" href="wg.css" media="all" /> 
<TABLE width="100%"><TR><TD><font size=+2>Word generator </font><I>(v1.1) </I></TD><TD class="hdr_info"><font style="font-family: tahoma;" size=-1> <A HREF="wg-updates.html">Last updates</A>&nbsp;&nbsp;&nbsp;&nbsp;
	<A HREF="wg_note.pdf">Info</A> </font></TD></TR></TABLE>
</head><body>
<HR>
  <DIV  id="wait" style="display:none" ><IMAGE height=50 src=/wait.gif></DIV>
  <?php echo ($errmsg); ?>

  <FORM METHOD=POST id=wg_form TARGET="word_f" ACTION="wg.php" enctype="multipart/form-data">
  <TABLE   width="100%" cols=4 class=option_table>
    <TR><TD> <A HREF="#abstr_factor_hint" class="hint_a">Abstraction factor</A></TD>
  <TD rowspan=5>
<div style="margin-bottom:4px; font-size:80%">

  <span class="ui-icon ui-icon-signal" style="float:left;"></span>

  <A HREF="#confidence_multiplier_hint">Confidence</A> dependent multipliers, %&nbsp;<A HREF="javascript:oneeq();">100%</A>
  <A HREF="javascript:reseteq();">Reset</A>

</div><div id="eq">
<div id="eq_red">
  <div>
  <span>0</span>
  <input name="eq0" type=text  style="margin-left: -10px; width=20pt; float=top;">
  </div>
  <div>
  <span>10</span>
  <input name="eq1" type=text  style="margin-left: -10px; width=20pt; float=top;">
  </div>
</div>
<div id="eq_yellow">
  <div>
  <span>40</span>
  <input name="eq2" type=text  style="margin-left: -10px; width=20pt; float=top;">
  </div>
  <div>
  <span>70</span>
  <input name="eq3" type=text  style="margin-left: -10px; width=20pt; float=top;">
  </div>
</div>
<div id="eq_green">
  <div>
  <span>100</span>
  <input name="eq4" type=text  style="margin-left: -10px; width=20pt; float=top;">
  </div>
  <div>
  <span>100</span>
  <input name="eq5" type=text  style="margin-left: -10px; width=20pt; float=top;">
  </div>
</div>
</div>

</TD><TD class="rack_handle" rowspan="5"><A HREF="wg_note.pdf" ><img src="handle.png" TITLE="How does it work?" ALT="How does it work?"></HREF>
</TD></TR>
<TR><TD>
  <TABLE width=90%><TR>
  <TD>0.0</TD><TD>0.1</TD><TD>0.2</TD><TD>0.3</TD><TD>0.5</TD><TD>0.7</TD><TD>1.0</TD><TD>1.5</TD>
</TR><TR>
  <TD><INPUT TYPE="RADIO" NAME="abstr_factor" VALUE="0.0" /></TD>
  <TD><INPUT TYPE="RADIO" NAME="abstr_factor" VALUE="0.1" /></TD>
  <TD><INPUT TYPE="RADIO" NAME="abstr_factor" VALUE="0.2" /></TD>
  <TD><INPUT TYPE="RADIO" NAME="abstr_factor" VALUE="0.3" checked /></TD>
  <TD><INPUT TYPE="RADIO" NAME="abstr_factor" VALUE="0.5" /></TD>
  <TD><INPUT TYPE="RADIO" NAME="abstr_factor" VALUE="0.7" /></TD>
  <TD><INPUT TYPE="RADIO" NAME="abstr_factor" VALUE="1.0" /></TD>
  <TD><INPUT TYPE="RADIO" NAME="abstr_factor" VALUE="1.5" /></TD>
  </TR>
  </TABLE>
</TD></TR>
<TR><TD><SPAN id="subsequent_multiplier_label"><HREF="#subsequent_multiplier_hint" style="font-size:80%;font-style:italic;" class="hint_a">Subsequent phoneme multiplier</A></SPAN></TD></TR>
<TR><TD><SPAN id="subsequent_multiplier">
  <INPUT TYPE="RADIO" NAME="subsequent_multiplier" VALUE="0" />0.0
  <INPUT TYPE="RADIO" NAME="subsequent_multiplier" VALUE="0.3" checked />0.3
  <INPUT TYPE="RADIO" NAME="subsequent_multiplier" VALUE="0.5" />0.5
  <INPUT TYPE="RADIO" NAME="subsequent_multiplier" VALUE="0.7" />0.7
  <INPUT TYPE="RADIO" NAME="subsequent_multiplier" VALUE="1" />1.0
  </SPAN></TD>
 </TR>
</TABLE>
  <TABLE width=100% cols=4 style="margin-top:8px;" class=option_table>
  <TR><TD>  <A href="#max_ng_hint" class="hint_a"> N-Gram size</A></TD>
  <TD colspan=2>Generated Word Count (max.<INPUT TYPE="number" name="max_syl" value=10 min=1 style="width:3em;"/>&nbsp;<A href="#syl_hint" class="hint_a">syllables</A>
)
  </TD>
  </TD><TD class="rack_handle" rowspan="6"><A HREF="wg_note.pdf"><img src="handle.png" TITLE="How does it work?" ALT="How does it work?"></HREF>
  </TR>
  <TR><TD>
  <INPUT TYPE="RADIO" NAME="max_ng" VALUE="3" onchange="onmax_ng();"/>  3
  <INPUT TYPE="RADIO" NAME="max_ng" VALUE="4" onchange="onmax_ng();"/>  4
  <INPUT TYPE="RADIO" NAME="max_ng" VALUE="5" onchange="onmax_ng();" checked />  5
  </TD><TD colspan=2>
  <INPUT TYPE="RADIO" NAME="count" VALUE="100" checked />  100
  <INPUT TYPE="RADIO" NAME="count" VALUE="200" />  200
  <INPUT TYPE="RADIO" NAME="count" VALUE="500" />  500
  <INPUT TYPE="RADIO" NAME="count" VALUE="1000" />  1000
  <INPUT TYPE="RADIO" NAME="count" VALUE="2000" /> 2000
  <INPUT TYPE="RADIO" NAME="count" VALUE="4000" /> 4000
  </TD></TR>
<!-------  <TR><TD>
  Pick
------->  </TD><TD>
  <A href="#dict_word_hint" class="hint_a">Dictionary words</A>
  </TD><TD>
    <A href="#score_hint" class="hint_a">Probability Score</A>
  </TD></TR>
<!-------  <TR><TD>
  <INPUT TYPE="RADIO" NAME="pick" VALUE="top" />Top
<INPUT TYPE="RADIO" NAME="pick" VALUE="200" checked/>Random
------->  </TD><TD>
<INPUT TYPE="CHECKBOX" ID="allow_hit" NAME="allow_hit" VALUE="1" onchange="onallow_hit();"/><LABEL for="allow_hit"> Allow </LABEL>
  <SPAN id="mark_hit_span" style="flow:left;"><INPUT TYPE="CHECKBOX" ID="mark_hit" NAME="mark_hit" VALUE="1" checked/><LABEL for="mark_hit"> Mark(#)</LABEL></span>
  </TD><TD>
  <INPUT TYPE="TEXT" NAME="min_score" style = " width: 5em"; VALUE="0" />Min
  <INPUT TYPE="CHECKBOX" NAME="display_score" VALUE="1" />Display
  <INPUT TYPE="CHECKBOX" NAME="sort_score" VALUE="1" />Sort by
  <INPUT TYPE="CHECKBOX" NAME="pick_top" VALUE="1" />Pick top

  </TD></TR>
  <TD colspan=2>
  <A href="#source_dict_hint" class="hint_a">Dictionary file</A>
  </TD>
    <TD>
  </TD></TR><TR><TD colspan=3>
    <SPAN id=dict_client><INPUT TYPE="FILE" NAME="dict_file" ID="dict_file"/>
  </SPAN>&nbsp;
  <INPUT TYPE=CHECKBOX NAME=dict_side ID=dict_side onchange="ondict_side();"  VALUE=server>
  sample
  <SPAN id=dict_server>:&nbsp;<INPUT TYPE=SUBMIT VALUE="View" class=small_btn NAME=view_sample><select NAME="dict_sample" ID="dict_sample">
  <?php
  foreach ($sample_dict as $i=>$n) {
	  $name =preg_replace('/_/',' ',$n);
	  echo "<option value=$i>$name</option>";
  } ?>
  </select></SPAN> 
  </TD>
  </TR>
  
  </TABLE>
<DIV class=hint id=abstr_factor_hint>
		<h3>Abstraction Factor</h3>
		<h4>Every generated word is produced according to the probability score of prefix continuation.
		 To determine this score it considers not just concrete prefixes found in the source dictionary but also abstracted ones
		 (where one or more phonemes are substituted with abstracted symbols like the consonant, the vowel and the frequent vowel.)
		 However, abstracted patterns yield lower scores than concrete ones, in order to make concrete prefixes preferable.
		 The Abstraction Factor defines a multiplicator applied to the score when we turn one phoneme position into an abstracted symbol.
		 The higher the abstraction factor is, the richer variety of words we obtain but their phonotactics may become less similar to that of the original dictionary.
		<BR><div align=center><A HREF="#" class=hint_a>Close</A></DIV></h4></DIV> 
<DIV class="cover" onclick="window.location.href='#';"></DIV>
<DIV class=hint id=subsequent_multiplier_hint>
		<h3>Subsequent phoneme multiplier</h3>
		<h4>Each time we increase the abstracted phonemes count in our probability calculations we apply this additional multiplier to the <A HREF="#abstr_factor_hint">Abstraction Factor.</A><BR>
		Example: let Abstraction factor be 0.5 and Subsequent phoneme multiplier be 0.7 .<BR>
		Then, {'a','b','c','d'} 4-gram in generated word sheel will be considered the same probable as {'a','b','c','d'} one in the source dictionary,
		 plus 0.5 times as probable as {'a',&lt;any consonant&gt;,'c','d'} and  {'a',&lt;frequent vowel&gt;'c','d'}
		 plus (0.5*0.7=0.35) times as probable as &lt;frequent vowel&gt;,&lt;any consonant&gt;,'c','d'}.
		<BR>See also <A HREF=confidence_multiplier_hint>Confidence dependent multiplier</A>
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
<DIV class="cover" onclick="window.location.href='#';"></DIV>
<DIV class=hint id=score_hint>
		<h3>Probability Score</h3>
		<h4>The Probability Score is an internal metric of a word appearance likelihood. (In the current implementation it's not necessary equal to the probability itself.)
		You may use the following controls to deal with the Score:
		<UL>
		<LI><Strong>Pick top</Strong> - run a flow that hunts for top scored words rather than making random 'shots'. When using it, please note that the word collection will not tend to show much variety in the length as well as in overall flavor.</LI> 
		<LI><Strong>Sort by</Strong> - sort the output by word Scores</LI>
		<LI><Strong>Display</Strong> - display the Score next to every generated word like <I>word (0.0001)</I></LI>
		<LI><Strong>Min</Strong> - filter out generated words having the Score value less than specified in this field</LI>
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

  </FORM><BR>
<iframe name="word_f" onload="$('#wait').hide('slow');" id="word_f" width=100% height=300>
  </IFRAME><BR>
  <DIV class="footer-note">
  Word generator does not store any data that you upload.
  <I><HR></DIV>
  <DIV CLASS="footer">Word generator is written by Andreas Scherbakov as part of a project run by Nick Thieberger with funding from ARC Future Fellowship (FT140100214)
  </I></DIV>

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

onallow_hit();
ondict_side();

</script>

</BODY></HTML>
<?php exit();} ?>

<?php echo $errmsg; ?>
<TABLE>
<?php $coln = 3;
  $j = $coln;
  $v = $_SESSION['produced_no'];
  //echo ("prod no = $v");
  $gwc = count($_SESSION['generated_words'][$v]);
  for ($i=0; $i < $gwc; $j+=$coln)
    {
	    echo "<TR>";
	    for (; $i < $j; ++$i)
    {
	    if (preg_match("/^Warning:/", $_SESSION['generated_words'][$v][$i])) {
		     echo "</TR><TR class=\"warnings\" style=\"background-color: #ffff00;\"><TD style=\"border-width: 2pt;\" colspan=$coln><span class=\"warnings\" style=\"background-color: #ffff00;\">".$_SESSION['generated_words'][$v][$i]."</SPAN></TD></TR><TR>";
		     }
	    else if (preg_match("/ #/", $_SESSION['generated_words'][$v][$i])) {
		     echo "<TD>".$_SESSION['generated_words'][$v][$i]."</TD>";}
		     else {
		     echo "<TD><div class=\"dict_hit\">".$_SESSION['generated_words'][$v][$i]."</div></TD>";}
	}
		echo "</TR>";
	}

 ?>
 </TABLE>

