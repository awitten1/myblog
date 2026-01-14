import { BlogPosts } from 'app/components/builtin/posts'

export default function Page() {
  return (
    <section className="container">
      <h1 className="section-title tracking-tighter">
        Andrew Witten's Blog
      </h1>
      <div className="intro">
        <p>
        Hi! I'm Andrew Witten.
        I’m a PhD student at the University of Maryland interested in database systems,
        CPU performance, and systems in general. I previously worked as a software engineer at
        MotherDuck and MongoDB. This blog is a place for me to write about those
        technical topics and anything else I happen to be thinking about.
        For fun, I enjoy running, going to the gym, and playing chess.
        </p>
        <img className="intro-photo" src="/photo.jpeg" alt="Me at Old Rag." />
      </div>
      <div className="section-content">
        <BlogPosts />
      </div>
    </section>
  )
}
