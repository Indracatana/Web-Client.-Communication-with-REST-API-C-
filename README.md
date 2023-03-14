# Web-Client.-Communication-with-REST-API-C-
• The Web Client receives input commands from the user and sends HTTP Requests to the REST API server that simulates an online library
• Used JWT Tokens to securely transmit information between the client and the server

--------------------------------------------------------------------------------------------------------------------------------------

requests.c
-compute_get_request: am adaugat in mesajul trimis catre server si token-ul de autorizare
-compute_post_request: am adaugat in mesajul trimis catre server si token-ul de autorizare
-compute_delete_request: am adaugat aceasta functie pentru a trimite server-ului un request
de tip DELETE, avand elementele: host, url, token, cookies, cookies_count.

client.c
In aceasta sursa, intr-un while citesc comenzile de la tastatura si le interpretez astfel:
•	register: citesc de la tastatura username-ul si parola utilizatorului si le adaug 
intr-un obiect json, trimitandu-le apoi intr-un POST request pentru autentificare catre server.  
•	login: citesc de la tastatura username-ul si parola utilizatorului, le adaug intr-un
obiect json si le trimit intr-o cerere de tip POST pentru logare catre server. Daca nu 
primesc mesaj eroare de la server, extrag cookie-ul de autentificare si campul pentru 
verificarea statusului de conectare de pe site il fac true.
•	 enter_library: verific daca clientul s-a logat inainte de a da aceasta comanda; daca
da, trimit request de tip GET pentru acces la librarie, iar daca acesta s-a efectuat cu succes
extrag din mesajul primit de la server token-ul de autentificare.
•	get_books: trimit request de tip GET pentru vizualizarea cartilor, iar daca acesta s-a
efectuat cu succes extrag raspunsul de la server intr-un array de obiecte json, afisand id-ul
si titlul pentru fiecare carte.
•	get_book: verific daca id-ul din comanda este un numar; daca acesta respecta formatul, 
trimit cerere de tip GET pentru a vizualiza una dintre cartile din biblioteca, iar daca aceasta
s –a efectuat cu succes extrag mesajul intr-un array de un obiect json si afisez informatiile
dorite despre cartea ceruta.
•	add_book: citesc campurile cartii pe care clientul doreste sa o adauge, verificand daca
numarul de pagini respecta formatul dorit; daca da, adaug intr-un obiect de tip json campurile
 cartii si il trimit printr-un request  de tip POST catre server.
•	delete_book:  verific daca id-ul din comanda este un numar; daca acesta respecta 
formatul, trimit cerere de tip DELETE pentru cartea specificata din biblioteca.
•	logout: verific daca clientul este logat; daca da, trimit cerere de tip GET pentru
delogare catre server, iar apoi sterg cookie-ul si token-ul clientului, iar campul pentru 
verificare a statusului de conectare de pe site il fac fals.
•	exit: dau break din while-ul din care citeam comenzile.

Pentru fiecare comanda primita de la tastatura, mai putin cea de exit, deschid conexiunea cu 
server-ul, iar dupa ce trimit request-ul necesar, interpretez mesajul primit drept raspuns de 
la server. Daca mesajul este null, inseamna ca nu am primit eroare de server, mai putin in cazul
get_books si get_book(in care primim mesaj de la server cu informatiile cerute despre librarie, 
iar mesajul este null daca libraria este goala) si in cazul enter_library in care primim token-ul 
de autentificare de la server.
La final, eliberez memoria alocata dinamic.
