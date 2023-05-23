  text	   data	    bss	    dec	    hex	filename
  99784	    312	 130760	 230856	  385c8	build/ch.elf
  
  sign
  text	   data	    bss	    dec	    hex	filename
  60616	    312	 130760	 191688	  2ecc8	build/ch.elf
  
  sign+keygen
  text	   data	    bss	    dec	    hex	filename
 126956	    312	 130760	 258028	  3efec	build/ch.elf
 
  workbuff réduit
  text	   data	    bss	    dec	    hex	filename
 126956	    312	 130760	 258028	  3efec	build/ch.elf
 
 Gestion du port série chibistudio
  1. Retire l'adaptateur USB/UART s'il est actuellement inséré. Il faut installer un driver pour l'adaptateur sur ta machine windows:
https://www.silabs.com/documents/public/software/CP210x_Windows_Drivers.zip
 2. Normalement, quand tu insères maintenant l'adaptateur, la led rouge s'allume et ça devrait être reconnu comme un port COM. Pour le vérifier et avoir le numéro du port COM: touche windows + x => Gestionnaire de périphériques => Ports (COM et LPT) => Silicon Labs CP210x... (COM5)
 3. Dans Eclipse (ChibiStudio), Ctrl+Alt+T ouvre un terminal, puis clique sur le petit terminal (icône la plus à gauche des icônes à droites de la petite fenêtre), choisis Serial Terminal et saisis les bonnes valeurs: COM5, 38400,8,None,1
C'est décrit ici, (attention tu n'as rien à installer sur ChibiStudio, tout y est déjà) : https://dzone.com/articles/using-serial-terminal-and-com-support-in-eclipse
 4. Vers la ligne 90 du main.c: (tu peux aussi réduire les délais sur le clignotement et mesurer les perfos sur la vérification de la signature)
chprintf((BaseSequentialStream *)&SD2,"Crypto_sign: Delta cycle count: %lu - Delta time: %lu\r\n", cycle_counter, timer);
 5. branche le TXD sur PA3 et RXD sur PA2 de la carte STM. Si tu inverses c'est pas grave mais tu ne verras rien s'afficher!
 6. Build/Flash/run... Tadaaaaa! Ca t'affiche les données...
  