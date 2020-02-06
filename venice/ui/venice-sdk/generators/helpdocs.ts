import * as fs from 'fs';
import * as rmdir from 'rimraf'
import { writeTemplate, walk, ensureFile, getSwagger, regexQuote } from './utils';
import * as path from 'path';
import * as generator from '@pensando/swagger-ts-generator'
import * as showdown from 'showdown';

export interface Config {
  // Base directory to look for md files
  helpDir: string,
  // Directory to write temp files to
  tempDir: string,
  htmlDir: string,
  // base path to inject to relative html links
  htmlLinkBase: string,
}


export function generateHelpDocs(config: Config) {
  if (fs.existsSync(config.tempDir)){
    // Delete all contents
    rmdir.sync(config.tempDir);
  }
  if (fs.existsSync(config.htmlDir)){
    // Delete all contents
    rmdir.sync(config.htmlDir);
  }
  fs.mkdirSync(config.htmlDir,  { recursive: true });
  
  // generate swagger map
  const options = getSwagger();
  
  // Recursively search all files for md
  let results = walk(config.helpDir, (file) => { return file.endsWith('.md') && !file.includes("README")});
  results = results.map( (x) => {
    return path.relative(path.join(process.cwd(), config.helpDir), x)
  });

  // Generate doc options
  results.forEach( (file) => {
    options.files.push({
      input: path.join(config.helpDir, file),
      output: path.join(config.tempDir, file),
      filePath: file,
    })
  })
  generator.generateDocs(options)

  // Generate link ref
  const converter = new showdown.Converter({tables: true, metadata: true})
  const refMap = {};
  options.files.forEach( (entry) => {
    const md = fs.readFileSync(entry.output, 'utf8');
    converter.makeHtml(md);
    const metadata = converter.getMetadata();
    let id = metadata.id;
    if (id == null) {
      console.error("id was null for file " + entry.output)
    }
    id = metadata.id.replace(/\s/g, '')
    id = id.replace(/\r?\n|\r/g, '')
    refMap[id] = path.join(config.htmlLinkBase, entry.filePath.split('.').slice(0, -1).join('.')) + '.html';
  });

  // Write link manifest file
  const outputFile = path.join(config.htmlDir, 'linkMap.ts');
  writeTemplate('generate-help-link-ts.hbs', refMap, outputFile)

  // Replace reference links in md files
  var linkReplacer = {
    type: 'lang',
    filter: function (text: string, converter, options) {
      Object.keys(refMap).forEach( (key) => {
        text = text.replace(new RegExp(regexQuote('(%' + key + ')'), 'g'), '(' + refMap[key] + ')');
      });
      text = text.replace(/\/images\//g, path.join(config.htmlLinkBase, '/images/'))
      return text;
    }
  };
  converter.addExtension(linkReplacer, 'LinkReplacer');

  options.files.forEach( (entry) => {
    const md = fs.readFileSync(entry.output, 'utf8');
    const html = converter.makeHtml(md);
    const output = path.join(config.htmlDir, entry.filePath.split('.').slice(0, -1).join('.')) + '.html';
    ensureFile(output, html);
  });

  // Copy all other files over to webapp
  results = walk(config.helpDir, (file) => { return !file.endsWith('.md') });
  results = results.map( (x) => {
    return path.relative(path.join(process.cwd(), config.helpDir), x)
  });
  results.forEach( (file) => {
    const output = path.join(config.htmlDir, file);
    fs.mkdirSync(path.dirname(output),  { recursive: true });
    fs.copyFileSync(path.join(config.helpDir, file), output);
  });

  // clean up tmp
  if (fs.existsSync(config.tempDir)){
    // Delete all contents
    rmdir(config.tempDir, {maxBusyTries: 20}, () => {});
  }
}

