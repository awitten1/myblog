import { BlogPosts } from 'app/components/builtin/posts'

export const metadata = {
  title: 'Blog',
  description: 'Read my blog.',
}

export default function Page() {
  return (
    <section className="container">
      <h1 className="section-title tracking-tighter">My Blog</h1>
      <BlogPosts />
    </section>
  )
}
