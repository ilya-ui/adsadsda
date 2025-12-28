import React, { useState } from 'react';
import { useOS } from '../context/OSContext';
import { apps } from '../apps/registry';

const Desktop: React.FC = () => {
    const { openApp } = useOS();
    const [selectedIconId, setSelectedIconId] = useState<string | null>(null);

    // Simple grid layout
    return (
        <div
            className="absolute inset-0 z-0 p-4 grid grid-cols-[repeat(auto-fill,minmax(96px,1fr))] grid-rows-[repeat(auto-fill,minmax(112px,1fr))] gap-2 content-start justify-items-center mb-12"
            onClick={() => setSelectedIconId(null)}
        >
            {apps.map((app) => (
                <div
                    key={app.id}
                    className={`flex flex-col items-center justify-center p-2 rounded w-24 h-28 cursor-default group transition-colors duration-100 ${selectedIconId === app.id ? 'bg-white/20 border border-white/30' : 'hover:bg-white/10 border border-transparent'
                        }`}
                    onClick={(e) => {
                        e.stopPropagation();
                        setSelectedIconId(app.id);
                    }}
                    onDoubleClick={() => openApp(app.id)}
                >
                    <div className="w-12 h-12 mb-2 relative flex items-center justify-center">
                        {/* Icon Background (optional, mimicking some win11 icons) */}
                        {/* <div className="absolute inset-0 bg-blue-500 blur-xl opacity-20 rounded-full"></div> */}
                        <app.icon className="w-full h-full text-white filter drop-shadow-md" strokeWidth={1.5} />
                    </div>
                    <span className="text-white text-xs text-center drop-shadow-md line-clamp-2 select-none">
                        {app.title}
                    </span>
                </div>
            ))}
        </div>
    );
};

export default Desktop;
