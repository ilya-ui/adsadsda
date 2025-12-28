import type { LucideIcon } from 'lucide-react';

export interface AppConfig {
    id: string;
    title: string;
    icon: LucideIcon;
    component: React.ComponentType<any>;
    defaultWidth?: number;
    defaultHeight?: number;
    color?: string; // Icon background color
}

export interface WindowInstance {
    id: string;
    appId: string;
    title: string;
    isMinimized: boolean;
    isMaximized: boolean;
    zIndex: number;
    position?: { x: number; y: number };
    size?: { width: number; height: number };
}
