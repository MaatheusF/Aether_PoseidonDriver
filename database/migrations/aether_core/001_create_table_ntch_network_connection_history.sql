/*Cria a tabela 'ntch_network_connection_history' no schema 'aether_core' para armazenar o historico de conexões via Network */
CREATE TABLE IF NOT EXISTS aether_core.ntch_network_connection_history (
    ntch_id SERIAL PRIMARY KEY,
    ntch_identify VARCHAR(20),
    ntch_client_ip INET NOT NULL,
    ntch_connected_date TIMESTAMPTZ DEFAULT NOW()
);

COMMENT ON column aether_core.ntch_network_connection_history.ntch_id IS 'Codigo sequencial da tabela';
COMMENT ON column aether_core.ntch_network_connection_history.ntch_identify IS 'Identificador do tipo string fornecido pelo cliente durante a conexão';
COMMENT ON column aether_core.ntch_network_connection_history.ntch_client_ip IS 'Endereço IPV4 no Cliente que originou a conexão';
COMMENT ON column aether_core.ntch_network_connection_history.ntch_connected_date IS 'Data/Hora em que a conexaõ foi realizada';