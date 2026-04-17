import createMDX from "@next/mdx";

/** @type {import('next').NextConfig} */
const nextConfig = {
  serverExternalPackages: ["duckdb", "@duckdb/node-bindings", "@duckdb/node-api"],
  outputFileTracingIncludes: {
    "/blog/[slug]": ["./code/**/*.csv"],
  },
};

const withMDX = createMDX({
  options: {
    remarkPlugins: [
      ["remark-math", {}],
      ["remark-frontmatter", "yaml"],
      ["remark-mdx-frontmatter", { name: "metadata" }],
    ],
    rehypePlugins: [["rehype-katex", {}]],
  },
});

export default withMDX(nextConfig);
