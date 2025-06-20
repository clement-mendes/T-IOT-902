# INDEX.JS

This file contains the main components of the API. It includes the necessary routes to send data to the database, so it can be used by Metabase.

# DB.JS

This file connects the API to the database using the PG module.

The `pg` module is the official PostgreSQL module for Node.js. It allows the Node.js application to connect to a PostgreSQL database in order to:

- Execute SQL queries (SELECT, INSERT, UPDATE, etc.)
- Manage connections
- Receive query results
- Handle transactions

The "Pool" enables connection reuse instead of opening a new one for each request.
