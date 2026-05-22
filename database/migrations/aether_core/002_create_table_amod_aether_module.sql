/*Cria a tabela 'amod_aether_module' no schema 'aether_core' contendo a lista de modulos do Sistema */
CREATE TABLE IF NOT EXISTS aether_core.amod_aether_module (
    id SERIAL PRIMARY KEY,
    module_name VARCHAR(50),
    schema_name VARCHAR(50),
    create_date TIMESTAMPTZ DEFAULT NOW(),
    active BOOLEAN
);

COMMENT ON TABLE aether_core.amod_aether_module IS 'Contem a lista de modulos do Sistema';
COMMENT ON column aether_core.amod_aether_module.id IS 'Codigo sequencial da tabela';
COMMENT ON column aether_core.amod_aether_module.module_name IS 'Nome do modulo';
COMMENT ON column aether_core.amod_aether_module.schema_name IS 'Nome do schema do banco de dados utilizado pelo modulo';
COMMENT ON column aether_core.amod_aether_module.create_date IS 'Data/Hora em que o modulo foi adicionado';
COMMENT ON column aether_core.amod_aether_module.active IS 'Status definindo se o modulo está ativo ou inativo';