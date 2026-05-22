/* Cria uma role 'role_admin' que contem as permissões para usuario administrador */
CREATE ROLE role_admin WITH SUPERUSER CREATEDB CREATEROLE INHERIT LOGIN REPLICATION;
/* Cria uma role 'role_application' que contem as permissões para usuarios da aplicação Aether*/
CREATE ROLE role_application WITH LOGIN;

/* Cria um usuario chamado 'aether_daemon' para ser usado para pela aplicação */
CREATE USER aether_daemon WITH PASSWORD 'favero10' IN ROLE role_application;
/* Cria um usuario chamado 'aethermanutencao' para ser usado para manutenção do banco de dados */
CREATE USER aethermanutencao WITH PASSWORD 'favero10' IN ROLE role_admin;