/*
** scope_gl.h : Scoped Push/Pop macros of common OpenGL calls to avoid state leaks
**
** IDEA:
** Control the opengl state-machine by putting as many calls as possible into push/pop scopes.
** Using these scopes consistently should make state leaks impossible and makes the entire current
** state for a draw call obvious, since all the state can be set/unset right before the call:
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
** More push/pop functions to add:
**    glEnableVertexAttribArray/glDisableVertexAttribArray
**    glBindRenderbuffer + related functions
**    glActiveTexture() : glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTextureUnit);
**    glScissor
**
** Possible improvements:
** - There should be an easy way to turn off restoring of states (since that requires expensively querying via glGetInteger, etc.).
**   Instead, unset the state without restoring the old state (i.e. glUseProgram(0) instead of glUseProgram(old)).
** - Rename all macros from scope_gl_use_program to scope_glUseProgram to mirror the underlying API call. The macro arguments should
**   also be identical to the passed parameters, so that we can convert any scope_... to a simple call to the OpenGL API by removing
**   'scope_' and appending ';'.
** - We could add calls to glError() at the end of every scope
** - Avoid warning about shadowing variables when putting scope macros on the same line
*/

/* helper macros */
#define TOKEN_PASTE(a, b) a##b
#define CONCAT(a,b) TOKEN_PASTE(a,b)
#define UQ(name) CONCAT(name, __LINE__) /* unique identifier */

#define scope_begin_end_var(begin, end, var) \
    for (int UQ(var) = (begin, 0); (UQ(var) == 0); (UQ(var) += 1), end)

#define scope_glUseProgram(id) \
    for (GLint UQ(prog), UQ(i) = (glGetIntegerv(GL_CURRENT_PROGRAM, &UQ(prog)), glUseProgram(id), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glUseProgram(UQ(prog))))
    //scope_begin_end_var(glUseProgram(id), glUseProgram(0), prog) // version that does not restore state

#define scope_glBindVertexArray(vao) \
    for (GLint UQ(old_vao), UQ(i) = (glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &UQ(old_vao)), glBindVertexArray(vao), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glBindVertexArray(UQ(old_vao))))
    //scope_begin_end_var(glBindVertexArray(vao), glBindVertexArray(0), vertarr) // version that does not restore state

/* NOTE: new name */
#define scope_glBindTexture2D(tex_id) \
    for (GLint UQ(old_tex), UQ(i) = (glGetIntegerv(GL_TEXTURE_BINDING_2D, &UQ(old_tex)), glBindTexture(GL_TEXTURE_2D, tex_id), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glBindTexture(GL_TEXTURE_2D, UQ(old_tex))))
    //scope_begin_end_var(glBindTexture(GL_TEXTURE_2D, tex_id), glBindTexture(GL_TEXTURE_2D, 0), texbind) // version that does not restore state

/* NOTE: new name */
#define scope_glBindArrayBuffer(vbo) \
    for (GLint UQ(old_vbo), UQ(i) = (glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &UQ(old_vbo)), glBindBuffer(GL_ARRAY_BUFFER, vbo), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glBindBuffer(GL_ARRAY_BUFFER, UQ(old_vbo))))
    //scope_begin_end_var(glBindBuffer(GL_ARRAY_BUFFER, vbo), glBindBuffer(GL_ARRAY_BUFFER, 0), arrbuf)  // version that does not restore state

/* GL_DEPTH_TEST, GL_BLEND, ... */
#define scope_glEnable(enumval) \
    for (GLint UQ(old_flag), UQ(i) = (glGetBooleanv(enumval, (GLboolean*) &UQ(old_flag)), glEnable(enumval), 0); \
         (UQ(i) == 0); (UQ(i) += 1, (UQ(old_flag) ? glEnable(enumval) : glDisable(enumval))))
    //scope_begin_end_var(glEnable(enumval), glDisable(enumval), glenable) // version that does not restore state

/* TODO: untested */
#define scope_glBindFramebuffer(fbo) \
    for (GLint UQ(old_fbo), UQ(i) = (glGetIntegerv(GL_FRAMEBUFFER_BINDING, &UQ(old_fbo)), glBindFramebuffer(GL_FRAMEBUFFER, fbo), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glBindFramebuffer(GL_FRAMEBUFFER, UQ(old_fbo))))
    //scope_begin_end_var(glBindFramebuffer(GL_FRAMEBUFFER, fbo), glBindFramebuffer(GL_FRAMEBUFFER, 0), old_fbo) // version that does not restore state

/* TODO: untested */
/* NOTE: does not restore previously attached texture, could maybe be achieved with glGetFramebufferAttachmentParameter
 * see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetFramebufferAttachmentParameter.xhtml */
// attachment = {GL_COLOR_ATTACHMENT0,GL_DEPTH_ATTACHMENT,GL_STENCIL_ATTACHMENT}
#define scope_glFramebufferTexture2D(fbo,attachment,tex) \
    scope_begin_end_var(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex, 0), \
                        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D,   0, 0), fbotex)

/* TODO: untested */
/* NOTE: does not restore previously attached texture, could maybe be achieved with glGetFramebufferAttachmentParameter
 * see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetFramebufferAttachmentParameter.xhtml */
// attachment = {GL_COLOR_ATTACHMENT0,GL_DEPTH_ATTACHMENT,GL_STENCIL_ATTACHMENT}
#define scope_glFramebufferRenderbuffer(fbo,attachment,renderbuffer) \
    scope_begin_end_var(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer), \
                        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, 0), fborenbuf)

#define scope_glViewport(x,y,w,h) \
    for (GLint UQ(view)[4], UQ(i) = (glGetIntegerv(GL_VIEWPORT, UQ(view)), glViewport(x, y, w, h), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glViewport(UQ(view)[0], UQ(view)[1], UQ(view)[2], UQ(view)[3])))

#define scope_glClearColor(r,g,b,a) \
    for (GLfloat UQ(clear)[4], UQ(i) = (glGetFloatv(GL_COLOR_CLEAR_VALUE, UQ(clear)), glClearColor(r,g,b,a), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glClearColor(UQ(clear)[0], UQ(clear)[1], UQ(clear)[2], UQ(clear)[3])))

#define scope_glBlendFunc(src,dst) \
    for (GLint UQ(s), UQ(d), UQ(i) = (glGetIntegerv(GL_BLEND_SRC, &UQ(s)), glGetIntegerv(GL_BLEND_DST, &UQ(d)), glBlendFunc(src, dst), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glBlendFunc(UQ(s), UQ(d))))

#define scope_glBlendEquation(eq) \
    for (GLint UQ(e), UQ(i) = (glGetIntegerv(GL_BLEND_EQUATION, &UQ(e)), glBlendEquation(eq), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glBlendEquation(UQ(e))))

/* TODO: untested */
#define scope_glCullFace(mode) \
    for (GLint UQ(m), UQ(i) = (glGetIntegerv(GL_CULL_FACE_MODE, &UQ(m)), glCullFace(mode), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glCullFace(UQ(m))))

/* TODO: untested */
#define scope_glFrontFace(orient) \
    for (GLint UQ(fo), UQ(i) = (glGetIntegerv(GL_FRONT_FACE, &UQ(fo)), glFrontFace(orient), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glFrontFace(UQ(fo))))

/* TODO: untested */
/* NOTE: new name */
#define scope_glBindSSBO(ssbo, binding) \
    for (GLint UQ(old_ssbo), UQ(i) = (glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &UQ(old_ssbo)), glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo), glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, UQ(old_ssbo)), glBindBuffer(GL_SHADER_STORAGE_BUFFER, UQ(old_ssbo))))
    // scope_begin_end_var((glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo), glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo)), (glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, 0), glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0)), ssbo)

// ext = {i,f,fv,iv,Iiv,Iuiv}
/* TODO: use cross-platform typeof() */
#define scope_glTex2DParameter(ext,param,val) \
    for (typeof(val) UQ(t2d), UQ(i) = (glGetTexParameter##ext##v(GL_TEXTURE_2D, param, &UQ(t2d)), glTexParameter##ext(GL_TEXTURE_2D, param, val), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glTexParameter##ext(GL_TEXTURE_2D, param, UQ(t2d))))

/* for pushing and popping uniform values */
/* NOTE: these are extremely wasteful */
#define scope_glUniformMatrix4fv(matrix,name) \
    for (GLint UQ(prog), UQ(j) = (glGetIntegerv(GL_CURRENT_PROGRAM, &UQ(prog)),0); (UQ(j) == 0); UQ(j) += 1) \
    for (GLfloat UQ(mat)[16], UQ(i) = (glGetUniformfv(UQ(prog), glGetUniformLocation(UQ(prog), name), UQ(mat)), glUniformMatrix4fv(glGetUniformLocation(UQ(prog), name), 1, GL_FALSE, matrix), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glUniformMatrix4fv(glGetUniformLocation(UQ(prog), name), 1, GL_FALSE, UQ(mat))))
#define scope_glUniformfv(val,name) \
    for (GLint UQ(prog), UQ(j) = (glGetIntegerv(GL_CURRENT_PROGRAM, &UQ(prog)),0); (UQ(j) == 0); UQ(j) += 1) \
    for (GLfloat UQ(old_val), UQ(i) = (glGetUniformfv(UQ(prog), glGetUniformLocation(UQ(prog), name), &UQ(old_val)), glUniform1f(glGetUniformLocation(UQ(prog), name), val), 0); \
         (UQ(i) == 0); (UQ(i) += 1, glUniform1f(glGetUniformLocation(UQ(prog), name), UQ(old_val))))
