import { PipeTransform, Pipe } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';

@Pipe({
    name: 'safeStyle'
})

export class SafeStylePipe implements  PipeTransform {
    name: string;
    pure?: boolean;

    constructor(private sanitizer: DomSanitizer) { }
    transform(value) {
        return this.sanitizer.bypassSecurityTrustStyle(value);
    }
}
