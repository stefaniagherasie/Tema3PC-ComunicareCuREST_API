# Tema3PC-ComunicareCuREST_API
[(Tema3 Protocoale de Comunicatii (2019-2020, seria CB)] 

Scopul temei este realizarea un client care sa interactioenze cu un REST API, 
implementand un client web care sa interactioneze cu un server. Aplicatia
ii permite clientului sa interactioneze cu serverul printr-o serie de comenzi
date de la tastatura.

## Implementare
Am pornit de la structura din laboratorul 10, citind comenzi de la tastatura 
intr-o structura repetitiva. Pentru fiecare tip de comanda se deschide 
conexiunea si se construieste un mesaj de tip `GET`, `POST` sau `DELETE`.

In `request.c` se afla implementarea acestor cereri, respectand formatul `HTTP`,
adaugand inclusiv headerele necesare(headerul de Authorization) si cookies.
Pentru cererile POST, datele care trebuie transmise sunt sub forma unui `JSON`
(am modificat scheletul ca body_data sa fie un char*, nu char** si am adaugat
ca parametru un token necesar pentru obtinerea accessului).

Pentru fiecare comanda am testat cu `check_error()` eventualele erori aparute,
extrase din raspunsul de la server. Am afisat mesaje de reusita si informatiile
cerute daca comanda a fost realizata cu success. S-au tratat si cazuri de
eroare aditionale (introducerea de informatii invalide, login cand clientul
e deja logat, page_count sau id contin caractere care nu sunt cifre sau sunt 
numere negative).

- Pentru comanda `register` am format un JSON cu `username` si `password`, citite
de la tastatura, testand daca datele sunt introduse corect. Am folosit parsarea
unui json in functia `compute_json_register_login()` si am obtinul json-ul ca
string, fiind trimis mai departe cu POST la server. 
- Pentru comanda
de `login` se extrage de aceasta data si token-ul necesar pentru a demonstra
autentificarea si cookie-urile. 
- Pentru comanda `enter_library` am trimis o cerere GET si am extras token-ul
care demonstreaza accessul. Acest token JWT va fi pus in corpul cererilor 
viitoare in header-ul de "Authorization Bearer".
- Pentru `get_books` se trimite, de asemenea, o cerere de tip GET, si se obtine
lista de carti din biblioteca. Lista de carti este afisata intr-un format mai
frumos cu `show_books_pretty()`, folosind parsarea cu JSON.

- Pentru `get_book` am format path-ul necesar pentru a extrage cartea cu id-ul 
dat, folosind functia form_path, care verifica si daca inputul e corect.
Informatiile despre carte primite in urma apelului GET au fost afisate intr-un
format mai user-friendly, folosind din nou parsarea cu JSON. 
- Pentru `delete_book`
s-a utilizat aceeasi functie de formare a path-ului, dar efectuand de aceasta 
data o cerere DELETE.

- Pentru `add_book` am format JSON-ul care contine toate informatiile despre carte,
verificand corectitudinea inputului de la tastatura. Se trimite JSON-ul cu
POST la server.

- Pentru `logout` se trimite cererea de dezabonare si se sterg token-urile 
salvate si cookie-urile, pentru a nu putea accesa informatii din biblioteca,
desi userul nu mai este logat.

- Comanda `exit` elibereaza spatiul alocat si inchide clientul.

## Mentiuni

- Cand se greseste un parametru la introducerea de informatii(de ex la login sau 
add_book), nerespectandu-se formatul, comanda trebuie introdusa de la inceput.
- Pentru parsarea inputului in format JSON astfel incat sa trimitem o cerere valida
la server am folosi functiile parson de la: https://github.com/kgabis/parson
