import { useEffect, useState } from "react";

export default function HelloAPI() {
  const [data, setData] = useState(null);

  useEffect(() => {
    fetch("/api/hello") // Use full URL if not using proxy
      .then((res) => res.json())
      .then((json) => setData(json))
      .catch(console.error);
  }, []);

  return (
    <div>
      <h1>API Example</h1>
      {data ? (
        <div>
          <p>Message: {data.message}</p>
          <p>Request count: {data.requestCount}</p>
        </div>
      ) : (
        <p>Loading...</p>
      )}
    </div>
  );
}
