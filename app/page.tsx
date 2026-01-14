import { BlogPosts } from 'app/components/builtin/posts'

export default function Page() {
  return (
    <section className="container">
      <h1 className="section-title tracking-tighter">
        Andrew Witten's Blog
      </h1>
      <div className="intro">
        <p>
          I'm into low-level systems, performance optimization, and explaining how things work under the hood.
          Currently exploring binary search vs. linear search metrics, DuckDB, and performance-driven blog design.
        </p>
        <img className="intro-photo" src="/photo.jpeg" alt="Me at Old Rag." />
      </div>
      <div className="section-content">
        <BlogPosts />
      </div>
    </section>
  )
}
