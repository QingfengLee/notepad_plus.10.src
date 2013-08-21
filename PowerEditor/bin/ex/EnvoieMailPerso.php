<?php
$destinataire = "Gaelle.Retout@METRO.FR,Yolande.Diaz@METRO.FR,abdeslam.el.kadiri@fr.abnamro.com,f.berbou@oberthurcs.com";
$expediteur   = "mouloud.bendaia@fr.nurun.com";
$reponse      = $expediteur;

echo "Ce script envoie un mail au format HTML … $destinataire";
$codehtml=
"Fouad dors devant son pc !";
mail($destinataire,
     "RE: Email au format HTML",
     $codehtml,
     "From: $expediteur\r\nReply-To: $reponse\r\nContent-Type: text/html; charset=\"iso-8859-1\"\r\n");
?>
