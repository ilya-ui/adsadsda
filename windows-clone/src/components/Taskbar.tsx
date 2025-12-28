import React, { useState, useEffect } from 'react';
import { useOS } from '../context/OSContext';
import { apps } from '../apps/registry';
import { Wifi, Battery, Volume2, ChevronUp } from 'lucide-react';
import { format } from 'date-fns';

const Taskbar: React.FC = () => {
    const { toggleStartMenu, isStartMenuOpen, activeWindowId, windows, minimizeWindow, focusWindow } = useOS();
    const [currentTime, setCurrentTime] = useState(new Date());

    useEffect(() => {
        const timer = setInterval(() => setCurrentTime(new Date()), 1000);
        return () => clearInterval(timer);
    }, []);

    // Filter apps to show on taskbar (Pinned + Open)
    // For simplicity, showing all available apps + indication if open
    const pinnedApps = apps;

    const handleAppClick = (appId: string) => {
        const window = windows.find(w => w.appId === appId);
        if (!window) {
            // Not open, cannot open directly from here in this simple version unless I pass openApp
            // Wait, I should access openApp from useOS
            // let's grab it
        } else {
            if (window.isMinimized || activeWindowId !== window.id) {
                focusWindow(window.id);
            } else {
                minimizeWindow(window.id);
            }
        }
    };

    const { openApp } = useOS();

    return (
        <div className="absolute bottom-0 left-0 right-0 h-12 bg-win-taskbar backdrop-blur-md z-50 flex items-center justify-between px-4 border-t border-white/20">

            {/* Start Button & Centered Icons */}
            <div className="absolute left-1/2 transform -translate-x-1/2 flex items-center space-x-2 h-full">

                {/* Start Button */}
                <button
                    className={`p-2 rounded hover:bg-white/40 transition-all ${isStartMenuOpen ? 'bg-white/40' : ''} active:scale-95`}
                    onClick={toggleStartMenu}
                >
                    <img
                        src="https://upload.wikimedia.org/wikipedia/commons/e/e1/Windows_11_logo.svg"
                        alt="Start"
                        className="h-6 w-6 drop-shadow-sm"
                    />
                </button>

                {/* Pinned Apps */}
                {pinnedApps.map(app => {
                    const isOpen = windows.some(w => w.appId === app.id);
                    const isActive = windows.some(w => w.appId === app.id && w.id === activeWindowId && !w.isMinimized);

                    return (
                        <button
                            key={app.id}
                            className={`relative p-2 rounded hover:bg-white/40 transition-all group active:scale-95 ${isActive ? 'bg-white/30' : ''}`}
                            onClick={() => isOpen ? handleAppClick(app.id) : openApp(app.id)}
                        >
                            <app.icon className={`h-6 w-6 transition-transform ${isOpen ? 'text-blue-600' : 'text-gray-700'}`} />

                            {/* Dot indicator for open apps */}
                            {isOpen && (
                                <div className={`absolute bottom-0.5 left-1/2 transform -translate-x-1/2 w-1.5 h-1.5 rounded-full transition-all ${isActive ? 'bg-blue-600 w-3' : 'bg-gray-400'}`} />
                            )}

                            {/* Tooltip (simple) */}
                            <span className="absolute -top-10 left-1/2 transform -translate-x-1/2 bg-gray-800 text-white text-xs px-2 py-1 rounded opacity-0 group-hover:opacity-100 transition-opacity whitespace-nowrap pointer-events-none">
                                {app.title}
                            </span>
                        </button>
                    )
                })}
            </div>

            {/* System Tray (Right) */}
            <div className="flex items-center space-x-2 h-full">
                <div className="hidden sm:flex items-center hover:bg-white/40 p-1 rounded px-2 space-x-1 cursor-default transition-colors">
                    <ChevronUp size={16} />
                </div>

                <div className="flex items-center hover:bg-white/40 p-2 rounded space-x-3 cursor-default transition-colors">
                    <Wifi size={16} />
                    <Volume2 size={16} />
                    <Battery size={16} />
                </div>

                <div className="flex flex-col items-end justify-center hover:bg-white/40 px-2 py-1 rounded cursor-default transition-colors h-full text-right leading-none space-y-0.5 min-w-[70px]">
                    <span className="text-xs font-medium">{format(currentTime, 'h:mm aa')}</span>
                    <span className="text-[10px] text-gray-600">{format(currentTime, 'd/M/yyyy')}</span>
                </div>

                {/* Show Desktop Nook */}
                <div className="w-1.5 h-full opacity-0 hover:opacity-100 border-l border-gray-300 ml-2"></div>
            </div>

        </div>
    );
};

export default Taskbar;
