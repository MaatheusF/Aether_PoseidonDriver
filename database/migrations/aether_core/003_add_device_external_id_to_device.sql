/*Altera a tabela 'amod_aether_module' no schema 'aether_core' adicionando uma nova coluna chamada 'external_id'*/
ALTER TABLE aether_core.amod_aether_module ADD COLUMN external_id VARCHAR(20);
ALTER TABLE aether_core.amod_aether_module ADD CONSTRAINT external_id_unique_key UNIQUE (external_id);

COMMENT ON column aether_core.amod_aether_module.external_id IS 'Codigo externo do modulo';