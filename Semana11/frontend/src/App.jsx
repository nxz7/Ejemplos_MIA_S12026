import { useMemo, useState } from 'react';

const apiUrl = import.meta.env.VITE_API_URL || 'http://localhost:3001';

export default function App() {
  const [commands, setCommands] = useState('');
  const [output, setOutput] = useState('');
  const [loading, setLoading] = useState(false);
  const [importedFile, setImportedFile] = useState('');

  const canRun = useMemo(() => commands.trim().length > 0 && !loading, [commands, loading]);

  async function handleImport(event) {
    const file = event.target.files?.[0];
    if (!file) return;

    const text = await file.text();
    setImportedFile(file.name);
    setCommands(text);
  }

  async function handleRun() {
    if (!commands.trim()) return;

    setLoading(true);
    setOutput('comandos!!!');

    try {
      const response = await fetch(`${apiUrl}/api/run`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ commands })
      });

      const data = await response.json();

      if (!response.ok || !data.ok) {
        const message = data.error || data.errorOutput || '[ERROR FRONTEND]ejecutar comandos';
        setOutput(`ERROR\n${message}\n\n${data.output || ''}`);
        return;
      }

      setOutput(data.output || 'NO HAY SALIDA');
    } catch (error) {
      setOutput(`ERROR\n${error instanceof Error ? error.message : '[ERROR FRONTEND]No se pudo conectar al backend.'}`);
    } finally {
      setLoading(false);
    }
  }

  return (
    <main className="page">
      <section className="card">
        <h1> ARCHIVOS - PROYECTO</h1>

        <label htmlFor="commands">Comandos</label>
        <textarea
          id="commands"
          value={commands}
          onChange={(e) => setCommands(e.target.value)}
          placeholder='LOS COMANDOS!!!!'
        />

        <div className="actions">
          <label className="file-btn" htmlFor="script-file">
            Importar script
          </label>
          <input
            id="script-file"
            type="file"
            accept=".txt,.mia,.script"
            onChange={handleImport}
          />

          <button onClick={handleRun} disabled={!canRun}>
            {loading ? 'LOADING' : 'Ejecutar'}
          </button>
        </div>

        {importedFile && <p className="file-name">Archivo importado: {importedFile}</p>}

        <label htmlFor="output">Salida</label>
        <pre id="output">{output}</pre>
      </section>
    </main>
  );
}
