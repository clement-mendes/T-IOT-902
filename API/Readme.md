# INDEX.JS

Ce fichier contient les éléments principaux de l'API, il va notamment contenir les routes nécessaires pour envoyer des données en base de données. Pour qu'elles soient utilisées par Metabase.

# DB.JS

Ce fichier permet de connecté l'API à la base de donnée grâce au module PG.

Le module pg est le module officiel PostgreSQL pour Node.js. Il te permet de connecter l'application Node.js à une base de données PostgreSQL pour :

exécuter des requêtes SQL (SELECT, INSERT, UPDATE, etc.)
- gérer les connexions
- recevoir des résultats de requêtes
- manipuler des transactions

Le "Pool" permet de réutiliser des connexions au lieu d’en ouvrir une à chaque requête.
