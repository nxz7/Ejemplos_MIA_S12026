import cors from 'cors';
import express from 'express';
import fs from 'node:fs';
import path from 'node:path';
import { spawn } from 'node:child_process';
import { fileURLToPath } from 'node:url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
const port = process.env.PORT || 3001;
// Permitir CORS solo desde el frontend
const frontendOrigin = process.env.FRONTEND_ORIGIN || 'http://localhost:5173';

app.use(cors({ origin: frontendOrigin }));
app.use(express.json({ limit: '1mb' }));

const classRoot = path.resolve(__dirname, '..');
const cliPath = path.join(classRoot, 'clase11');

function runAnalyzer(commands) {
  return new Promise((resolve, reject) => {
    if (!fs.existsSync(cliPath)) {
      reject(new Error('[ERROR] No se encontro el binario clase11. Primero compila en Semana11.'));
      return;
    }

    const child = spawn(cliPath, [], { cwd: classRoot });
    let stdout = '';
    let stderr = '';
    let finished = false;

    const timeout = setTimeout(() => {
      if (!finished) {
        child.kill('SIGTERM');
      }
    }, 20000);

    child.stdout.on('data', (chunk) => {
      stdout += chunk.toString();
    });

    child.stderr.on('data', (chunk) => {
      stderr += chunk.toString();
    });

    child.on('error', (error) => {
      clearTimeout(timeout);
      finished = true;
      reject(error);
    });

    child.on('close', (code) => {
      clearTimeout(timeout);
      finished = true;
      resolve({ code, stdout, stderr });
    });

    child.stdin.write(`${commands.trim()}\nexit\n`);
    child.stdin.end();
  });
}

app.get('/api/health', (_req, res) => {
  res.json({ ok: true, service: 'semana11-backend' });
});

app.post('/api/run', async (req, res) => {
  const commands = typeof req.body?.commands === 'string' ? req.body.commands : '';

  if (!commands.trim()) {
    res.status(400).json({ ok: false, error: '[ERROR] SE DEBE ENVIAR UN COMANDO' });
    return;
  }

  try {
    const result = await runAnalyzer(commands);
    const ok = result.code === 0;

    res.status(ok ? 200 : 500).json({
      ok,
      exitCode: result.code,
      output: result.stdout,
      errorOutput: result.stderr
    });
  } catch (error) {
    res.status(500).json({
      ok: false,
      error: error instanceof Error ? error.message : 'ERROR DESC - SERVER.JS'
    });
  }
});

app.listen(port, () => {
  console.log(`>>>> Semana11 backend escuchando en http://localhost:${port}`);
});
