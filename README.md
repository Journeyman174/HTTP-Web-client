# HTTP-Web-client

  Am implemetat  un client web care interactioneaza cu un server.
	Clientul este un program, client.c , care accepta comenzile specificate de tema 
de la tastatura si trimite in functie de comanda, cereri (request) catre server. Clientul 
functioneaza ca o interfata (UI) cu biblioteca virtuala de pe server.
	Dupa lansarea programului cu comanda ./client , se realizeaza o conexiune prin
socket, la adresa IP a serverului, pe portul 8080. Functia utilizata pentru trimiterea 
cererilor este send_to server() si cea pentru primirea raspunsului este receive_from_server() ,
ambele din helpers.c . Pasii in dialogul cu serverul sunt urmatorii : mai intai se creaza mesajul
cererii utilizand corespunzator functiile compute_post_request(), compute_get_request() si 
respectiv, compute_delete_request(). Apoi se transmite cu functia send_to_sever().  Raspunsul primit
in urma executarii functiei receive_from_server() este interpretat corespunzator comenzii date.
Aceste functii se gasesc in request.c 
	 Am ales pentru parsarea (interpretarea)  raspunsurilor primite de la server, in format JSON (Javascript Object Notation),
functiile din biblioteca parson.c . Cererile transmise catre server sunt memorate in variabile de tip pointer : char * message 
de dimensiune 4096 octeti si raspunsurile serverului  in  char * response de dimensiune 4096 octeti. 
	Comenzile se citesc printr-o bucla while pana introducerea comenzii exit.
	Ordinea comenzilor dintr-o sesiune de lucru este :
		1.  comanda login <nume>,  < password> 
				daca contul nu exista, utilizatorul este invitat sa isi creeze un nou cont cu ajutorul
comenzii register <nume>,  < password>
				dupa inregistrarea cu succes a contului, se intra in sesiune cu comanda 
login <nume>,  < password>
				dupa autentificarea cu succes (executia cu succes a comenzii login este memorata
in variabila int login =  1), se extrage din raspunsul serverului  cookie-ul de sesiune care este
un mesaj JSON extras cu functia basic_extract_json_response() din helpers.c  a carui parsare
se realizeaza utilizand json_parse_string() si json_value_get_object() din biblioteca parson.c .
Cookie-ul de sesiune se memoreaza in variabila char * cooky, care impreuna cu login = 1, confirma
logarea. 

		2. cerereea de acces la biblioteca - comanda ente_library
				in urma acceptarii cererii de catra server, se extrage
din raspunsul primit, token-ul JWT si se memoreaza in varaibila char * token_jwt .
				prezenta cookie-ului de sesiune cooky si a token-ului JWT token_jwt
este o confirmare a accesului la biblioteca virtuala, memorate in variabila int acces = 1.
		
		3. comanda get_books
				afiseaza informatiile ID si Titlu pentru toate cartile din biblioteca, daca exista.
				informatiile despre carti sunt extrase din  raspunsul serverului in format JSON,
utilizand functiile bibliotecii parson.c 
		
		4. comanda get_book
				intoarce informatii corespunzator id-ului cerut ca data de intrare : titlu,
autor,  gen,  nr. pagina. 
		
		5. comanda add_book
				permite adaugarea unei carti in biblioteca prin crearea unei date JSON, 
construita conform structurii de carte din datele introduse de utilizator.
				se face o validare la campul page_count, sa fie numeric.
		
		6. comanda delete_book
				sterge cartea cu id-ul introdus de catre utilizator, din biblioteca virtuala
		
		7. comanda logout
				permite iesirea din sesiune, acceptarea comenzii logout fiind memorata in 
variabila int logout = 1.
