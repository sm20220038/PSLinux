# PSLinux
> Ovaj C program je nalik ps funkciji u Bash-u rađen za projektni rad iz Arhitekture računara i Operativni sistema na Fakultetu Organizacionih Nauka
## Namena programa
> Program služi za ispisivanje detaljnih informacija o procesima na Linux operativnim sistemima. Čita podatke iz /proc direktorijuma i prikazuje ih u terminalu.
## Način korišćenja
### 1. Kompajliranje programa
>Kompilacija programa se vrši pozivom iz direktorijuma gde je projekat, sa komandne linije: ```make``` koji generiše konačan program.
### 2. Pokretanje programa
> Program se pokreće pomoću `./main [opcije]` gde su moguće opcije opisane u nastavku.
### 3. Opcije
> Program ima opciju help koja služi kao uputstvo za program, i njemu se pristupa kada se unese bilo šta osim sledećih opcija kao argument funkcije:
- [null] - Jednostavan prikaz trenutno aktivnih procesa
- a - Prikazuje sve procese koji imaju tty, uključujuci procese i drugih korisnika
- u - Prikazuje procese efektivnog korisnika
- x - Prikazuje procese bez kontrolisanja tty-a
- aux - Prikazuje sve procese sa korisnikom, bez kontrolisanja tty-a i pokazuje korišćenje procesora i memorije