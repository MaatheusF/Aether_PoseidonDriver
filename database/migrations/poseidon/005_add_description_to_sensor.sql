/*Altera a tabela 'sens_sensor' no schema 'poseidon' adicionando uma nova coluna chamada 'description'*/
ALTER TABLE poseidon.sens_sensor ADD COLUMN description VARCHAR(200);

COMMENT ON column poseidon.sens_sensor.description IS 'Descrição do Sensor';