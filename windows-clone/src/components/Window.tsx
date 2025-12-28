import React, { useRef } from 'react';
import Draggable from 'react-draggable';
import { X, Minus, Square } from 'lucide-react';
import { useOS } from '../context/OSContext';
import type { WindowInstance } from '../types';
import { apps } from '../apps/registry';
import { motion } from 'framer-motion';

interface WindowProps {
    window: WindowInstance;
}

const Window: React.FC<WindowProps> = ({ window }) => {
    const { closeWindow, minimizeWindow, maximizeWindow, focusWindow } = useOS();
    const app = apps.find(a => a.id === window.appId);
    const nodeRef = useRef(null);

    if (!app) return null;

    return (
        <Draggable
            handle=".window-titlebar"
            bounds="parent"
            nodeRef={nodeRef}
            onMouseDown={() => focusWindow(window.id)}
            disabled={window.isMaximized}
        >
            <motion.div
                ref={nodeRef}
                initial={{ scale: 0.95, opacity: 0 }}
                animate={{ scale: 1, opacity: 1, transition: { duration: 0.2 } }}
                exit={{ scale: 0.95, opacity: 0, transition: { duration: 0.15 } }}
                className={`absolute flex flex-col bg-white rounded-lg shadow-2xl overflow-hidden border border-gray-300 ${window.isMaximized ? 'inset-0 !transform-none !w-full !h-full rounded-none top-0 left-0' : ''}`}
                style={{
                    width: window.isMaximized ? '100%' : (app.defaultWidth || 600),
                    height: window.isMaximized ? '100%' : (app.defaultHeight || 400),
                    zIndex: window.zIndex,
                    display: window.isMinimized ? 'none' : 'flex'
                }}
            >
                {/* Title Bar */}
                <div
                    className="window-titlebar h-9 bg-gray-50 flex items-center justify-between px-2 select-none border-b border-gray-100"
                    onDoubleClick={() => maximizeWindow(window.id)}
                >
                    <div className="flex items-center space-x-2 flex-1">
                        <app.icon className="w-4 h-4 text-gray-600" />
                        <span className="text-xs text-gray-700 font-medium">{window.title}</span>
                    </div>

                    <div className="flex items-center space-x-0 h-full">
                        <button
                            onClick={(e) => { e.stopPropagation(); minimizeWindow(window.id); }}
                            className="w-10 h-full flex items-center justify-center hover:bg-gray-200 transition-colors text-gray-600"
                        >
                            <Minus size={14} />
                        </button>
                        <button
                            onClick={(e) => { e.stopPropagation(); maximizeWindow(window.id); }}
                            className="w-10 h-full flex items-center justify-center hover:bg-gray-200 transition-colors text-gray-600"
                        >
                            <Square size={12} />
                        </button>
                        <button
                            onClick={(e) => { e.stopPropagation(); closeWindow(window.id); }}
                            className="w-10 h-full flex items-center justify-center hover:bg-red-500 hover:text-white transition-colors text-gray-600 rounded-tr-lg" // specific border radius for top right only if not maximized?
                        >
                            <X size={14} />
                        </button>
                    </div>
                </div>

                {/* Content */}
                <div className="flex-1 relative overflow-auto">
                    <app.component />
                </div>
            </motion.div>
        </Draggable>
    );
};

export default Window;
