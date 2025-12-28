import type { AppConfig } from '../types';
import { FileText, Calculator, Globe, Terminal } from 'lucide-react';
import Notepad from './Notepad';

// Placeholder components for other apps
const VSCode = () => (
    <div className="h-full w-full bg-[#1e1e1e] text-white flex items-center justify-center">
        <div className="text-center">
            <Terminal className="w-16 h-16 mx-auto mb-4 text-blue-400" />
            <h1 className="text-2xl font-bold">VS Code</h1>
            <p className="text-gray-400">Environment Simulation</p>
        </div>
    </div>
);

const Edge = () => (
    <div className="h-full flex flex-col bg-white">
        <div className="h-8 bg-gray-100 flex items-center px-4 border-b space-x-2">
            <div className="flex space-x-1">
                <div className="w-2 h-2 rounded-full bg-red-400"></div>
                <div className="w-2 h-2 rounded-full bg-yellow-400"></div>
                <div className="w-2 h-2 rounded-full bg-green-400"></div>
            </div>
            <div className="flex-1 bg-white rounded-md h-6 border flex items-center px-2 text-xs text-gray-500">
                https://www.google.com
            </div>
        </div>
        <iframe src="https://www.bing.com" className="flex-1 w-full border-none" title="Browser" />
    </div>
);

const CalculatorApp = () => (
    <div className="h-full w-full bg-gray-100 flex items-center justify-center">
        <div className="text-center">
            <Calculator className="w-12 h-12 mx-auto mb-2" />
            <p>Calculator Placeholder</p>
        </div>
    </div>
);


export const apps: AppConfig[] = [
    {
        id: 'notepad',
        title: 'Notepad',
        icon: FileText,
        component: Notepad,
        defaultWidth: 600,
        defaultHeight: 400,
        color: '#0078d4'
    },
    {
        id: 'vscode',
        title: 'VS Code',
        icon: Terminal,
        component: VSCode,
        defaultWidth: 800,
        defaultHeight: 600,
        color: '#23a9f2'
    },
    {
        id: 'edge',
        title: 'Microsoft Edge',
        icon: Globe,
        component: Edge,
        defaultWidth: 900,
        defaultHeight: 600,
        color: '#00a4ef'
    },
    {
        id: 'calc',
        title: 'Calculator',
        icon: Calculator,
        component: CalculatorApp,
        defaultWidth: 320,
        defaultHeight: 480,
        color: '#e81123'
    }
];
