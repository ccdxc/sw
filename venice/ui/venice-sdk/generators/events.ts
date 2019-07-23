import * as utils from './utils';

export function generateEventTypesFile(inputFile: string, outputFile: string) {
  const events = utils.readJSON(inputFile)
  const templateFile = 'generate-eventtypes-ts.hbs';
  utils.writeTemplate(templateFile, events, outputFile);
}
