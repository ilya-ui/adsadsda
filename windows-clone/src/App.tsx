import { OSProvider } from './context/OSContext';
import Desktop from './components/Desktop';
import Taskbar from './components/Taskbar';
import StartMenu from './components/StartMenu';
import WindowManager from './components/WindowManager';

function App() {
  return (
    <OSProvider>
      <div className="relative w-screen h-screen overflow-hidden bg-hero bg-cover bg-center font-sans antialiased text-gray-900 select-none">

        {/* Desktop Area */}
        <Desktop />

        {/* Floating Windows Layer */}
        <WindowManager />

        {/* UI Overlays */}
        <StartMenu />

        {/* Taskbar - Always on top */}
        <Taskbar />

      </div>
    </OSProvider>
  );
}

export default App;
