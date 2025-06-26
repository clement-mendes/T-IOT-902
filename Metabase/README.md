# Installation and deployment of METABASE
Installation of Docker: https://docs.docker.com/get-started/get-docker/

## DEPLOYMENT OF DOCKER CONTAINERS
Download the Metabase container:

"docker pull metabase/metabase:latest"

## Download the Postgres container:

"docker pull postgres"

## Download the pgAdmin container:

"docker pull dpage/pgadmin4"

## Starting the containers

Start the Metabase container on port 3000:

"docker run -d -p 3000:3000 --name metabase metabase/metabase"

Start the Postgres container on port 5432 with credentials:

"docker run --name postgres_db -e POSTGRES_USER=admin -e POSTGRES_PASSWORD=admin -e POSTGRES_DB=mydatabase -p 5432:5432 -d postgres"

Start pgadmin4:

"docker run --name pgadmin-container -p 5050:80 -e PGADMIN_DEFAULT_EMAIL=user@domain.com -e PGADMIN_DEFAULT_PASSWORD=admin -d dpage/pgadmin4"

## View the database content via terminal

"docker exec -it postgres_db psql -U admin -d mydatabase"

## Connect the database to pgAdmin:

Start the containers and go to the indicated port on pgAdmin: "http://localhost:5050"

Enter the credentials defined above

Click on "Add Server" and enter the information we have from Postgres. For the hostname, retrieve the IP of the Postgres container using:

"docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' postgres_db"

In pgAdmin, run the following query to create the table that will receive our data:
"
CREATE TABLE sensors (
id SERIAL PRIMARY KEY,
sensor_id VARCHAR(10) NOT NULL,
temperature FLOAT NULL,
pressure FLOAT NULL,
airquality INT NULL,
sound FLOAT NULL,
created_at TIMESTAMP DEFAULT NOW()
);
"

## Add a DB to METABASE:

Go to the METABASE address, by default "http://localhost:3000/"

Click on "Add a database"

Once your database is added, you will be able to use the table you want to work with.


_____________________________________________________________________________________________________

VERSION FRANCAISE 

# Installation et déploiement de METABASE 

Installation du docker : https://docs.docker.com/get-started/get-docker/

## DEPLOIEMENT DES CONTAINERS DOCKER

Téléchargement du conteneur metabase : 

"docker pull metabase/metabase:latest"

Téléchargement du conteneur postgres : 

"docker pull postgres"

Téléchargement du conteneur pgadmin : 

"docker pull dpage/pgadmin4"

## Lancement des containers 

Lancement du container metabase sur le port 3000

"docker run -d -p 3000:3000 --name metabase metabase/metabase"

Lancement du container metabase sur le port 5432 en donnant les credentials

"docker run --name postgres_db -e POSTGRES_USER=admin -e POSTGRES_PASSWORD=admin -e POSTGRES_DB=mydatabase -p 5432:5432 -d postgres"

Lancement de pgadmin4 

"docker run --name pgadmin-container -p 5050:80 -e PGADMIN_DEFAULT_EMAIL=user@domain.com -e PGADMIN_DEFAULT_PASSWORD=admin -d dpage/pgadmin4"

## Voir le contenu de la database via le terminal 

"docker exec -it postgres_db psql -U admin -d mydatabase"

## connecter la databse à pgadmin :

Lancer les conteneurs et connecter vous au port indiquer sur pgamdin : "http://localhost:5050"

Rentrer les credentials enregistrer plus haut

Cliquer sur add serveur et renseigner les éléments que nous avons sur postgres, concernant le hostname il faut aller chercher l'IP de postgres 

"docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' postgres_db"

Dans pgadmin lancer la query suivant pour créer la table qui va recevoir nos données :
"
CREATE TABLE sensors (
  id SERIAL PRIMARY KEY,
  sensor_id VARCHAR(10) NOT NULL,
  temperature FLOAT NULL,
  pressure FLOAT NULL,
  airquality INT NULL,
  sound FLOAT NULL,
  created_at TIMESTAMP DEFAULT NOW()
);
""

## Ajouter une DB à METABASE :

Rendez vous sur l'adresse de METABASE, par défaut "http://localhost:3000/"

Cliquer sur ajouter une base de données 

Une fois que votre base de donnée est ajouter vous avez la possibilité d'utiliser la table que vous souhaitez utiliser.