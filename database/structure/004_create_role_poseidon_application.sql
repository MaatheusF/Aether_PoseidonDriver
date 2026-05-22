/* Cria um usuario chamado 'POSEIDON_APPLICATION' para ser usado para pela aplicação */
CREATE USER POSEIDON_APPLICATION WITH PASSWORD 'favero10' IN ROLE role_application;


GRANT ALL PRIVILEGES ON SCHEMA poseidon TO role_application;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA poseidon TO role_application;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA poseidon TO role_application;
ALTER DEFAULT PRIVILEGES IN SCHEMA poseidon GRANT SELECT, INSERT, UPDATE, DELETE ON TABLES TO role_application;
