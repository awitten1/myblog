import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  serverExternalPackages: ["duckdb", "@duckdb/node-bindings","@duckdb/node-api"],
};

export default nextConfig;
