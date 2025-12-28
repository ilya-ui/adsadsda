import React from 'react';
import { useOS } from '../context/OSContext';
import Window from './Window';
import { AnimatePresence } from 'framer-motion';

const WindowManager: React.FC = () => {
    const { windows } = useOS();

    return (
        <div className="absolute inset-0 pointer-events-none z-10 overflow-hidden pb-12">
            {/* pointer-events-none allows clicks to pass through to desktop if no window is hit. 
            However, Draggable needs pointer events. 
            We'll let children re-enable pointer events.
        */}
            <div className="relative w-full h-full pointer-events-auto">
                <AnimatePresence>
                    {windows.map((window) => (
                        <Window key={window.id} window={window} />
                    ))}
                </AnimatePresence>
            </div>
        </div>
    );
};

export default WindowManager;
