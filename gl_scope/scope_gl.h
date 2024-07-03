/*
** IDEA:
** Control the opengl state-machine by putting as many calls as possible into push/pop scopes.
** Using these scopes consistently should make state leaks impossible and makes the entire current
** for a draw call obvious, since all the state is set/unset right before the call:
**
**   scope_gl_use_program(toonshader)
**    scope_gl_bind_texture2d(spritesheet)
**     scope_gl_enable(GL_BLEND)
**      scope_gl_blend_func(GL_ONE, GL_SRC_ALPHA)
**       ...
**        scope_gl_clearcolor(0,0,0,0)
**   {
**       glDrawArrays(GL_TRIANGLES, 0, 6);
**   }
**
** Possible improvements:
** - There should be an easy way to turn off restoring of states (since that requires expensively querying via glGetInteger, etc.).
**   Instead, unset the state without restoring the old state (i.e. glUseProgram(0) instead of glUseProgram(old)).
*/

/* helper macros */
#define TOKEN_PASTE(a, b) a##b
#define CONCAT(a,b) TOKEN_PASTE(a,b)
#define UQ(name) CONCAT(name, __LINE__) /* unique identifier */

#define scope_begin_end_var(begin, end, var) \
    for (int UQ(var) = (begin, 0); (UQ(var) == 0); (UQ(var) += 1), end)

/*                                               */
/* NOTE: these do not restore the previous state */
/*                                               */
#define scope_gl_bind_array_buffer(vbo) \
    scope_begin_end_var(glBindBuffer(GL_ARRAY_BUFFER, vbo), glBindBuffer(GL_ARRAY_BUFFER, 0), arrbuf)

/* TODO probably doesn't compile because of the comma (?) */
#define scope_gl_bind_ssbo(ssbo, binding) \
    scope_begin_end_var((glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo), glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo)), (glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0), glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0)), ssbo)

#define scope_gl_bind_vertex_array(vao) \
    scope_begin_end_var(glBindVertexArray(vao), glBindVertexArray(vao), vertarr)

#define scope_gl_bind_texture2d(tex_id) \
    scope_begin_end_var(glBindTexture(GL_TEXTURE_2D, tex_id), glBindTexture(GL_TEXTURE_2D, 0), texbind)

/* GL_DEPTH_TEST, GL_BLEND, ... */
#define scope_gl_enable(enumval) \
    scope_begin_end_var(glEnable(enumval), glDisable(enumval), glenable)

/*                                                                          */
/* NOTE: these do restore previous state (by querying, which could be slow) */
/*                                                                          */
#define scope_gl_use_program(id) \
    for (GLint UQ(prog), UQ(i) = (glGetIntegerv(GL_CURRENT_PROGRAM, &UQ(prog)), glUseProgram(id), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glUseProgram(UQ(prog))))
    //scope_begin_end_var(glUseProgram(id), glUseProgram(0), prog) // version that does not restore state

#define scope_gl_viewport(x,y,w,h) \
    for (GLint UQ(view)[4], UQ(i) = (glGetIntegerv(GL_VIEWPORT, UQ(view)), glViewport(x, y, w, h), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glViewport(UQ(view)[0], UQ(view)[1], UQ(view)[2], UQ(view)[3])))
#define scope_gl_clearcolor(r,g,b,a) \
    for (GLfloat UQ(clear)[4], UQ(i) = (glGetFloatv(GL_COLOR_CLEAR_VALUE, UQ(clear)), glClearColor(r,g,b,a), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glClearColor(UQ(clear)[0], UQ(clear)[1], UQ(clear)[2], UQ(clear)[3])))

#define scope_gl_blend_func(src,dst) \
    for (GLint UQ(s), UQ(d), UQ(i) = (glGetIntegerv(GL_BLEND_SRC, &UQ(s)), glGetIntegerv(GL_BLEND_DST, &UQ(s)), glBlendFunc(src, dst), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glBlendFunc(UQ(s), UQ(d))))

#define scope_gl_blend_eq(eq) \
    for (GLint UQ(e), UQ(i) = (glGetIntegerv(GL_BLEND_EQUATION, &UQ(e)), glBlendEquation(eq), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glBlendEquation(UQ(e))))

#define scope_gl_cullface_mode(mode) \
    for (GLint UQ(m), UQ(i) = (glGetIntegerv(GL_CULL_FACE_MODE, &UQ(m)), glCullFace(mode), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glCullFace(UQ(m))))

#define scope_gl_frontface_orient(orient) \
    for (GLint UQ(fo), UQ(i) = (glGetIntegerv(GL_FRONT_FACE, &UQ(fo)), glFrontFace(orient), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glFrontFace(UQ(fo))))

// ext = {i,f,fv,iv,Iiv,Iuiv}
#define scope_gl_tex2d_param(ext,param,val) \
    for (typeof(val) UQ(t2d), UQ(i) = (glGetTexParameter##ext(GL_TEXTURE_2D, param, &UQ(t2d)), glTexParameter##ext(GL_TEXTURE_2D, param, val), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glTexParameter##ext(GL_TEXTURE_2D, param, UQ(t2d))))

/* not scope opening */
// ext = {1fv,2fv,3fv,4fv,1iv,2iv,3iv,4iv,Matrix4fv,...}
// NOTE: no error handling or logging
#define gl_upload_uniform(ext,prog,name,...) \
    scope_gl_use_program(prog) { \
        glUniform##ext(glGetUniformLocation(prog,name), __VA_ARGS__); \
    }

/*
  More possible push/pop functions include
     glEnableVertexAttribArray/glDisableVertexAttribArray
     glBindFramebuffer
     glActiveTexture() : glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTextureUnit);

     one could also add calls to glError() at the end of every scope
*/