import React, { createContext, useContext, useState, type ReactNode } from 'react';
import type { WindowInstance } from '../types';
import { apps } from '../apps/registry';

interface OSContextType {
    windows: WindowInstance[];
    activeWindowId: string | null;
    isStartMenuOpen: boolean;
    openApp: (appId: string) => void;
    closeWindow: (id: string) => void;
    minimizeWindow: (id: string) => void;
    maximizeWindow: (id: string) => void;
    focusWindow: (id: string) => void;
    toggleStartMenu: () => void;
}

const OSContext = createContext<OSContextType | undefined>(undefined);

export const OSProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
    const [windows, setWindows] = useState<WindowInstance[]>([]);
    const [activeWindowId, setActiveWindowId] = useState<string | null>(null);
    const [isStartMenuOpen, setIsStartMenuOpen] = useState(false);
    const [zIndexCounter, setZIndexCounter] = useState(10);

    const openApp = (appId: string) => {
        const app = apps.find((a) => a.id === appId);
        if (!app) return;

        // Optional: Check if single instance app is already open
        // For now, allow multiple instances

        const newWindow: WindowInstance = {
            id: `${appId}-${Date.now()}`,
            appId,
            title: app.title,
            isMinimized: false,
            isMaximized: false,
            zIndex: zIndexCounter + 1,
        };

        setWindows([...windows, newWindow]);
        setActiveWindowId(newWindow.id);
        setZIndexCounter(zIndexCounter + 1);
        setIsStartMenuOpen(false);
    };

    const closeWindow = (id: string) => {
        setWindows(windows.filter((w) => w.id !== id));
        if (activeWindowId === id) {
            // Focus the next top-most window
            const remaining = windows.filter((w) => w.id !== id);
            if (remaining.length > 0) {
                const sorted = [...remaining].sort((a, b) => b.zIndex - a.zIndex);
                setActiveWindowId(sorted[0].id);
            } else {
                setActiveWindowId(null);
            }
        }
    };

    const focusWindow = (id: string) => {
        if (activeWindowId === id) return;

        setWindows(windows.map(w =>
            w.id === id ? { ...w, zIndex: zIndexCounter + 1, isMinimized: false } : w
        ));
        setActiveWindowId(id);
        setZIndexCounter(zIndexCounter + 1);
    };

    const minimizeWindow = (id: string) => {
        setWindows(windows.map(w =>
            w.id === id ? { ...w, isMinimized: true } : w
        ));
        setActiveWindowId(null); // Deselect logic could be better
    };

    const maximizeWindow = (id: string) => {
        setWindows(windows.map(w =>
            w.id === id ? { ...w, isMaximized: !w.isMaximized } : w
        ));
        focusWindow(id);
    };

    const toggleStartMenu = () => {
        setIsStartMenuOpen(!isStartMenuOpen);
    };

    return (
        <OSContext.Provider
            value={{
                windows,
                activeWindowId,
                isStartMenuOpen,
                openApp,
                closeWindow,
                minimizeWindow,
                maximizeWindow,
                focusWindow,
                toggleStartMenu,
            }}
        >
            {children}
        </OSContext.Provider>
    );
};

export const useOS = () => {
    const context = useContext(OSContext);
    if (!context) {
        throw new Error('useOS must be used within an OSProvider');
    }
    return context;
};
