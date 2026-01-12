import { BlogPosts } from 'app/components/builtin/posts'

export default function Page() {
  return (
    <section className="container">
      <h1 className="section-title tracking-tighter">
        Technical Musings
      </h1>
      <p>
        I'm into low-level systems, performance optimization, and explaining how things work under the hood.
        Currently exploring binary search vs. linear search metrics, DuckDB, and performance-driven blog design.
      </p>
      <div className="section-content">
        <BlogPosts />
      </div>
    </section>
  )
}
