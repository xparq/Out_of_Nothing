import * as vscode from 'vscode';
import * as path from 'path';
import * as fs from 'fs';

export function activate(context: vscode.ExtensionContext) {
    const workspaceRoot = vscode.workspace.workspaceFolders?.[0]?.uri.fsPath || '.';

    const toolingRoot = path.resolve(workspaceRoot, 'tooling', 'edit', 'codium');
    const userDataDir = path.join(toolingRoot, 'user-data');
    const extensionsDir = path.join(toolingRoot, 'extensions');

    if (fs.existsSync(userDataDir)) {
        vscode.window.showInformationMessage(`Codium portable detected at: ${userDataDir}`);
    }

    context.subscriptions.push(
        vscode.commands.registerCommand('portableAware.hello', async (...args) => {
            vscode.window.showInformationMessage(`Hello from portable-aware extension! Args: ${JSON.stringify(args)}`);
        })
    );
}

export function deactivate() {}
