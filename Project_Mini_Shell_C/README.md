Projet MyShell

L'objectif de ce projet est de réaliser un mini-shell capable d'exécuter des programmes.
Celui-ci doit être capable de: 
	- D' exécuter plusieurs commande en une seule ligne
	- Gérer les wildcards
	- Différentes commandes personnalisées (cd, status, myls , myps)
	- Les redirections
	- Premier et arrière plans
	- Gestion des variables locales et d' environnement
	

Organisation


Il a été réalisé par Stephen LAURENT et Ouahab Fenniche.
En ce qui concerne notre organisation, nous avons choisis de nous répartir les taches selon l'enoncé du sujet. Par exemple, le séquencement pour Stephen et les wildcards pour Ouahab. (donc on a partager le projet 50 50 )

Nous n'avons pas rencontrés de problème concernant notre méthode de travail au cours du développement.


Travail effectué


Ce que nous avons pu réaliser:
	- Le séquencement
	- Changement de répertoire
	- Sortie de shell
	- Commande status
	- La redirection vers ou depuis un fichier

Pour les tubes, la gestion d'un fonctionne mais pas plusieurs par exemple ls|wc retourne la bonne valeur alors que pour ls|sort|wc, le wc et le sort ne liront pas sur les bonnes sorties.

Ceci étant dans le programme ce trouvant dans mysh_test

En ce qui concerne le reste, les wildcards, myls, myps , premier et arrière plan et les variables ont étaient codés mais nous n'avons pas réussi combinés avec le programme principal

Donc actuellement seul ce qui se trouve dans les dossiers peut etre testés, les fichiers en racine quant à eux servent d'essais pour regrouper le tout.

Il faudra donc par exemple executé ./myls dans myls_test pour voir le myls, ./myps dans myls_test pour voir le myps ou ./mysh dans mysh_test pour avoir notre shell. 


Concernant notre blague "
Stephen: tu as ouahab une blague pour le projet?
Ouahab: je t'ecoute
Stephen: moi aussi, je t'ecoute
Comme on s' écoute tous les deux, on se retrouve alors en interblocage....
