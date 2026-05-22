/*Cria a tabela 'jcrh_job_cron_history' no schema 'poseidon' contendo o historico das rotinas*/
CREATE TABLE IF NOT EXISTS poseidon.jcrh_job_cron_history (
	id SERIAL PRIMARY KEY,
	schedule_id INTEGER NOT NULL REFERENCES poseidon.jcrs_job_cron_schedule (id) ON DELETE CASCADE,
	run_at TIMESTAMP NOT NULL,
	status VARCHAR(20) NOT NULL, --RUNNING | SUCCESS | FAILED
	error_message VARCHAR(200),
	create_date TIMESTAMP DEFAULT NOW()
);

COMMENT ON TABLE poseidon.jcrh_job_cron_history IS 'Contem o historico de execução do agendamento de tarefas';
COMMENT ON column poseidon.jcrh_job_cron_history.id IS 'Codigo sequencial da tabela';
COMMENT ON column poseidon.jcrh_job_cron_history.schedule_id IS 'Codigo do agendamento na tabela jcrs_job_cron_schedule';
COMMENT ON column poseidon.jcrh_job_cron_history.run_at IS 'Data de execução do agendamento';
COMMENT ON column poseidon.jcrh_job_cron_history.status IS 'Status da execução';
COMMENT ON column poseidon.jcrh_job_cron_history.error_message IS 'Em caso de erro armazena a mensagem de retorno gerada';
COMMENT ON column poseidon.jcrh_job_cron_history.create_date IS 'Data/Hora em que o dado foi cadatrado';