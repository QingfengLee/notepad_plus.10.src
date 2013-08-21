/* HOU Chin Wu */

import java.io.*;
import java.net.*;

public class ClientFtp 
{
    /* donnees qui ne peuvent pas etre modifiees */
	final static int PORT = 6667;
    final static int SIZE = 1024;
    final static byte[] buf = new byte[SIZE];
	static FileInputStream fIn;
	static FileOutputStream fOut;
	static DatagramPacket donneesIn = new DatagramPacket(new byte[SIZE], SIZE);
	static DatagramPacket donneesOut;
	static InetAddress dest;
	
    public static void main(String[] args) throws Exception
	{
		if (args.length != 3)
		{
			System.out.println("\nSyntax : java ClientFtp serveur command fichier\n");
			System.exit(-1);
		}
        
		if (!(args[1].equals("GET"))&&(!args[1].equals("PUT")))
		{
			System.out.println("\nSoit \"PUT\", soit \"GET\", pas autrement.\n");
			System.exit(-1);
		}
		
		try {
			// Obetenir l'adresse a partir du nom de machine
        	dest = InetAddress.getByName(args[0]);
		} catch (UnknownHostException e) 
		  {
		  	  System.out.println ("Desole, hote non trouve. L'operation echoue");
			  System.exit(-1);
		  }
		// Creer le socket de commmunication
        DatagramSocket prise = new DatagramSocket(PORT);

		// Concatener les chaine de caracteres : 
		// soit "put+nomDeFichier", soit "get+nomDeFichier"
		String str = args[1].concat(args[2]);
		
		for (int i = 0 ; i < str.length() ; i++)
			buf[i] = (byte)str.charAt(i);
		
		buf[str.length()] = (byte)' ';
			
		// En cas de "put", le client envoie :
		// "put+NomDeFichier+" "+leContenuDeFichier", le blanc est
		// le separateur entre le nom du fichier et le contenu du fichier

		if (args[1].equals("PUT"))
		{
			try {
				donneesOut = new DatagramPacket(buf, buf.length, dest, PORT);
				donneesIn = new DatagramPacket(new byte[SIZE], SIZE);
				fIn = new FileInputStream(args[2]);
			
				// la longueur de fichier + la longueur de commande
				// + la longueur du nom du fichier + un blanc
				donneesOut.setLength(fIn.available()+4+args[2].length());
				fIn.read(buf, str.length()+1, fIn.available()-1);
				System.out.println ("la taille des donnees a envoyer est "+donneesOut.getLength());
				prise.send(donneesOut);
				
				System.out.println ("Le fichier \""+args[2]+"\" est parti!");
				prise.receive(donneesIn);
				int tailleDeDonnees = donneesIn.getLength();
				if ((tailleDeDonnees == 1)&&(donneesIn.getData()[0] == 0))
				{
					System.out.println("Bien, le fichier est arrive au serveur.");			
				}
			}
			catch (FileNotFoundException e)
			{
				System.out.println("Desole, le fichier a envoyer "+args[2]+" n'est pas trouve.");
				System.exit(-1);
			}
		}
		else // le cas de "GET"
		{
			donneesOut = new DatagramPacket(buf, buf.length, dest, PORT);

			prise.send(donneesOut);
			
			//donneesIn = new DatagramPacket(new byte[SIZE], SIZE);
			
			prise.receive(donneesIn);
			
			int tailleDeDonnees = donneesIn.getLength();

			// Virification : 
			// verifier si le fichier a recuperer se trouve dans le cote de serveur,
			// si oui, on le sauvegarde. Sinon, on quitte directement.
			
			if ((tailleDeDonnees == 1)&&(donneesIn.getData()[0] == -1))
			{
				System.out.println ("Zut! Le fichier n'y est pas. Tant pis!");
				System.exit(-1);
			}
			System.out.println ("J'ai recu le fichier \""+args[2]+"\" dont la taille est : "+ donneesIn.getLength());
			
			fOut = new FileOutputStream(args[2]);
			fOut.write(donneesIn.getData(), 0, tailleDeDonnees);
			fOut.flush();
			fOut.close();
		}
    }
}

