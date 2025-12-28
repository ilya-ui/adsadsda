import React, { useState } from 'react';

const Notepad: React.FC = () => {
    const [text, setText] = useState('Welcome to Windows 11 Web.\n\nType something...');

    return (
        <div className="h-full flex flex-col bg-white text-black font-mono text-sm">
            <div className="flex space-x-2 p-1 border-b border-gray-200 text-xs text-gray-600 bg-gray-50">
                <span className="hover:bg-gray-200 px-2 py-0.5 rounded cursor-pointer">File</span>
                <span className="hover:bg-gray-200 px-2 py-0.5 rounded cursor-pointer">Edit</span>
                <span className="hover:bg-gray-200 px-2 py-0.5 rounded cursor-pointer">Format</span>
                <span className="hover:bg-gray-200 px-2 py-0.5 rounded cursor-pointer">View</span>
                <span className="hover:bg-gray-200 px-2 py-0.5 rounded cursor-pointer">Help</span>
            </div>
            <textarea
                className="flex-1 p-2 outline-none resize-none border-none focus:ring-0"
                value={text}
                onChange={(e) => setText(e.target.value)}
            />
            <div className="h-6 bg-gray-100 border-t border-gray-200 flex items-center px-2 text-xs text-gray-500 justify-end">
                <span>UTF-8</span>
            </div>
        </div>
    );
};

export default Notepad;
