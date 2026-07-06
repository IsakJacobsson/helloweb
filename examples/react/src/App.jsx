import { BrowserRouter as Router, Routes, Route, Link } from "react-router-dom";
import Home from "./Home";
import About from "./About";
import HelloAPI from "./HelloAPI";

function App() {
  return (
    <Router>
      <nav style={{ marginBottom: "1rem" }}>
        <Link to="/" style={{ marginRight: "1rem" }}>Home</Link>
        <Link to="/about" style={{ marginRight: "1rem" }}>About</Link>
        <Link to="/hello">API Hello</Link>
      </nav>

      <Routes>
        <Route path="/" element={<Home />} />
        <Route path="/about" element={<About />} />
        <Route path="/hello" element={<HelloAPI />} />
      </Routes>
    </Router>
  );
}

export default App;
