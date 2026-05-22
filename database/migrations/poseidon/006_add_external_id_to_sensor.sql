/*Altera a tabela 'sens_sensor' no schema 'poseidon' adicionando uma nova coluna chamada 'external_id'*/
ALTER TABLE poseidon.sens_sensor ADD COLUMN external_id VARCHAR(20);
ALTER TABLE poseidon.sens_sensor ADD CONSTRAINT external_id_unique_key UNIQUE (external_id);

COMMENT ON column poseidon.sens_sensor.external_id IS 'Codigo externo do modulo';