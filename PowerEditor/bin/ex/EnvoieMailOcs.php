<?php
$destinataire = "f.berbou@oberthurcs.com,o.JAMETON@oberthurcs.com,v.FINKELSTEIN@oberthurcs.com,l.DACLIN@oberthurcs.com,f.DUMOULIN@oberthurcs.com,h.MOBARHAN@oberthurcs.com,m.GERMON@oberthurcs.com,c.FOUGEROUSE@oberthurcs.com,jm.BONNAND@oberthurcs.com,b.BOURGIER@oberthurcs.com,f.VANDELANNOOTE@oberthurcs.com,cw.HOU@oberthurcs.com,e.RICHARD@oberthurcs.com,f.CLUZET@oberthurcs.com";
$expediteur   = "f.ELISABETH@oberthurcs.com";
$reponse      = $expediteur;

echo "Ce script envoie un mail au format HTML … $destinataire";
$codehtml=
"<html><body>" .
"<h1>haha arrete tes connerries toi</h1>".
"<b><u>tu chier toi laisse don tranquille</u></b><br>" .
"d ailleur a quoi je me mele <font color=\"red\">couleurs</font>" .
"</body></html>";
mail($destinataire,
     "RE: personnel confidentiel, chute!!!",
     $codehtml,
     "From: $expediteur\r\nReply-To: $reponse\r\nContent-Type: text/html; charset=\"iso-8859-1\"\r\n");
?>
