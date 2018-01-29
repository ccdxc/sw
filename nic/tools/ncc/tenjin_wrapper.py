
import sys
import tenjin
import argparse


def render_template(out, name, context, templates_dir, prefix=None):
    """
    Render a template using tenjin.
    out: a file-like object
    name: name of the template
    context: dictionary of variables to pass to the template
    prefix: optional prefix for embedding (for other languages than python)
    """

    # support "::" syntax
    pp = [tenjin.PrefixedLinePreprocessor(prefix=prefix)
           if prefix else tenjin.PrefixedLinePreprocessor()]
    # disable HTML escaping
    template_globals = {"to_str": str, "escape": str}
    if templates_dir:
        engine = TemplateEngine(path=[templates_dir], pp=pp, cache=False)
    else:
        engine = TemplateEngine(pp=pp, cache=False)
    out.write(engine.render(name, context, template_globals))
    if 'KD_DICT' in context:
        return context['KD_DICT']
    if 'P4TBL_TYPES' in context:
        return context['P4TBL_TYPES']
    else:
        return None


# We have not found a use for this yet, so excude it from cov report
class TemplateEngine(tenjin.Engine):  # pragma: no cover
    def include(self, template_name, **kwargs):
        """
        Tenjin has an issue with nested includes that use the same local
        variable names, because it uses the same context dict for each level of
        nesting.  The fix is to copy the context.
        """
        frame = sys._getframe(1)
        locals = frame.f_locals
        globals = frame.f_globals
        context = locals["_context"].copy()
        context.update(kwargs)
        template = self.get_template(template_name, context, globals)
        return template.render(context, globals, _buf=locals["_buf"])

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--template", required=True, help="Template file") 
    parser.add_argument("--outfile", required=True, help="Output file") 
    parser.add_argument("--prefix", default="//::", help="Tenjin prefix") 
    parser.add_argument("--args", default=None, help="Args to pass to tenjin") 
    args = parser.parse_args()

    template = args.template
    out_file = args.outfile
    tenjin_prefix = args.prefix

    dic = {"args":args.args}
    with open(out_file, "w") as of:
        render_template(of, template, dic, None, prefix=tenjin_prefix)
