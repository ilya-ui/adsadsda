import React, { useRef, useEffect } from 'react';
import { useOS } from '../context/OSContext';
import { apps } from '../apps/registry';
import { Search, Power } from 'lucide-react';
import { motion, AnimatePresence } from 'framer-motion';

const StartMenu: React.FC = () => {
    const { isStartMenuOpen, toggleStartMenu, openApp } = useOS();
    const menuRef = useRef<HTMLDivElement>(null);

    // Close when clicking outside
    useEffect(() => {
        // Logic removed or handled by overlay
    }, [isStartMenuOpen]);

    return (
        <AnimatePresence>
            {isStartMenuOpen && (
                <>
                    {/* Overlay to close menu on click outside */}
                    <div className="fixed inset-0 z-40" onClick={toggleStartMenu}></div>

                    <motion.div
                        ref={menuRef}
                        initial={{ y: 20, opacity: 0, scale: 0.95 }}
                        animate={{ y: 0, opacity: 1, scale: 1 }}
                        exit={{ y: 20, opacity: 0, scale: 0.95 }}
                        transition={{ duration: 0.2 }}
                        className="absolute bottom-14 left-1/2 transform -translate-x-1/2 w-[640px] h-[700px] max-h-[85vh] bg-win-taskbar/95 backdrop-blur-xl border border-white/20 rounded-lg shadow-2xl z-50 flex flex-col p-6 space-y-6"
                    >
                        {/* Search Bar */}
                        <div className="relative">
                            <Search className="absolute left-3 top-2.5 text-gray-500 w-5 h-5" />
                            <input
                                type="text"
                                placeholder="Search for apps, settings, and documents"
                                className="w-full bg-[#f3f3f3] border border-gray-300 rounded-full py-2 pl-10 pr-4 text-sm focus:outline-none focus:ring-2 focus:ring-blue-500"
                            />
                        </div>

                        {/* Pinned Apps */}
                        <div className="flex-1 overflow-y-auto pr-2">
                            <div className="flex items-center justify-between mb-4">
                                <h3 className="text-sm font-semibold text-gray-700 ml-2">Pinned</h3>
                                <button className="text-xs bg-white px-2 py-1 rounded border border-gray-200 shadow-sm hover:bg-gray-50">All apps &gt;</button>
                            </div>

                            <div className="grid grid-cols-6 gap-4">
                                {apps.map(app => (
                                    <button
                                        key={app.id}
                                        className="flex flex-col items-center p-2 rounded hover:bg-white/50 transition-colors active:scale-95"
                                        onClick={() => openApp(app.id)}
                                    >
                                        <app.icon className="w-8 h-8 mb-2 text-gray-700" />
                                        <span className="text-xs text-center text-gray-700 font-medium line-clamp-1">{app.title}</span>
                                    </button>
                                ))}
                            </div>

                            <div className="mt-8 mb-4">
                                <h3 className="text-sm font-semibold text-gray-700 ml-2">Recommended</h3>
                            </div>
                            <div className="grid grid-cols-2 gap-4">
                                <div className="flex items-center space-x-3 p-2 hover:bg-white/50 rounded cursor-pointer">
                                    <div className="bg-blue-100 p-2 rounded text-blue-600"><FileIcon /></div>
                                    <div className="flex flex-col">
                                        <span className="text-xs font-semibold">Project Implementation.docx</span>
                                        <span className="text-[10px] text-gray-500">2h ago</span>
                                    </div>
                                </div>
                                <div className="flex items-center space-x-3 p-2 hover:bg-white/50 rounded cursor-pointer">
                                    <div className="bg-green-100 p-2 rounded text-green-600"><FileIcon /></div>
                                    <div className="flex flex-col">
                                        <span className="text-xs font-semibold">Budget 2026.xlsx</span>
                                        <span className="text-[10px] text-gray-500">Yesterday</span>
                                    </div>
                                </div>
                            </div>
                        </div>

                        {/* Footer / User Profile */}
                        <div className="h-12 border-t border-gray-300/50 flex items-center justify-between pt-2 px-4 -mx-6 bg-black/5 -mb-6 rounded-b-lg">
                            <div className="flex items-center space-x-3 hover:bg-white/40 p-2 rounded cursor-pointer transition-colors">
                                <div className="w-8 h-8 bg-blue-600 rounded-full flex items-center justify-center text-white font-bold text-xs">US</div>
                                <span className="text-xs font-semibold text-gray-700">User</span>
                            </div>

                            <button className="p-2 hover:bg-white/40 rounded transition-colors text-gray-700">
                                <Power size={20} />
                            </button>
                        </div>

                    </motion.div>
                </>
            )}
        </AnimatePresence>
    );
};

const FileIcon = () => (
    <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" className="lucide lucide-file-text w-5 h-5"><path d="M15 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V7Z" /><path d="M14 2v4a2 2 0 0 0 2 2h4" /><path d="M10 9H8" /><path d="M16 13H8" /><path d="M16 17H8" /></svg>
)

export default StartMenu;
