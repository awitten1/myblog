import Link from 'next/link'
import { formatDate, getBlogPosts } from 'app/blog/utils'

export function BlogPosts() {
  let allBlogs = getBlogPosts()

  return (
    <div>
      {allBlogs
        .sort((a, b) => {
          if (
            new Date(a.metadata.publishedAt) > new Date(b.metadata.publishedAt)
          ) {
            return -1
          }
          return 1
        })
        .map((post) => (
          <Link
            key={post.slug}
            className="post-item"
            href={`/blog/${post.slug}`}
          >
            <div className="post-row">
              <p className="post-date">
                {formatDate(post.metadata.publishedAt, false)}
              </p>
              <p className="post-title">
                {post.metadata.title}
              </p>
            </div>
          </Link>
        ))}
    </div>
  )
}
