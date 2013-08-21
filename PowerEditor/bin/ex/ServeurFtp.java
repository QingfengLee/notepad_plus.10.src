/* HOU Chin Wu */

import java.io.*;
import java.net.*;

class ServeurFtp 
{
    final static int PORT = 6667;
    final static int SIZE = 1024;
    final static byte[] buf = new byte[SIZE];
	static FileInputStream fIn;
	static FileOutputStream fOut;
	static DatagramPacket donneesIn, donneesOut;
	
    public static void main(String argv[]) throws Exception
	{
		System.out.println("");
		System.out.println("* Bienvenu au Tftp *");

		
        DatagramSocket prise = new DatagramSocket(PORT);
        System.out.println (prise.getLocalAddress());
        System.out.println (prise.getLocalPort());
        while (true)
		{
			donneesIn = new DatagramPacket(buf, SIZE);
			
			// se mettre a ecouter
           	prise.receive(donneesIn);
			
			byte[] donneesTab = donneesIn.getData();
			int i;
			for (i = 0 ; (i < donneesTab.length)&&((char)donneesTab[i] != ' ') ; i++);
			
			// En cas de "PUT"
	   		if ((new String(donneesTab, 0, i)).startsWith("PUT"))
			{
				// extaire le nom de fichier en ouvrant le fichier a ecrire 
				String nomFichier = new String(donneesTab).substring(3,i);
				fOut = new FileOutputStream(nomFichier);
				System.out.println ("le nom du fichier est "+"\""+nomFichier+"\"");
				fOut.write(donneesTab, i+1, donneesIn.getLength()-3-nomFichier.length()-1);
				
				// Fermer le fichier
				fOut.flush();
				fOut.close();
				System.out.println ("le fichier "+nomFichier+" est bien recu.");
				buf[0] = 0;
				donneesOut = new DatagramPacket(buf, buf.length, donneesIn.getAddress(), PORT);
				donneesOut.setLength(1);
				prise.send(donneesOut);
				System.out.println ("Le message pour signaler au client que j'ai recu le fichier est parti!");
			}
			else // En cas de "GET"
			{
				try {
					String nomFichier = new String(donneesIn.getData()).substring(3, i);
					System.out.println ("Recu get : " + "\""+nomFichier+"\"");
					fIn = new FileInputStream(nomFichier);
					int tailleDeFichier = fIn.available();
					System.out.println ("J'ai reussit a trouver le fichier "+nomFichier+" dont la longueur est : "+tailleDeFichier+". Je l'envoyerai a "+donneesIn.getAddress().toString());
					fIn.read(buf, 0, tailleDeFichier);
					donneesOut = new DatagramPacket(buf, buf.length, donneesIn.getAddress(), PORT);
					System.out.println ("la packet est cree.");
					donneesOut.setLength(tailleDeFichier);
					System.out.println ("setLength est fait.");
					prise.send(donneesOut);
					System.out.println ("Le fichier est parti!!!");
					fIn.close();
				}
				// Traitement d'erreur :
				// Si le fichier a envoyer ne se trouve pas de cote de serveur, 
				// le serveur envoie un message d'erreur comme reponse au client.
				// Et puis le serveur continue de boucler et d'attendre d'autre requete.
				catch (FileNotFoundException e)
				{
					System.out.println("Fichier non trouve.");
					buf[0] = -1;
					donneesOut = new DatagramPacket(buf, buf.length, donneesIn.getAddress(), PORT);
					donneesOut.setLength(1);
					prise.send(donneesOut);
					System.out.println ("Le message d'erreur est parti!!!");
					System.out.println ("J'attends d'autres requetes.");
				}
			}
        }	
	}
}

