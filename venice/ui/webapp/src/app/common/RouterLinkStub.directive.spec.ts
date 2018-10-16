import { Directive, Input, HostListener } from '@angular/core';

/**
 * Used for checking router links in unit tests
 * Taken from https://angular.io/guide/testing#components-with-routerlink
 *
 * const linkDes = fixture.debugElement.queryAll(By.directive(RouterLinkStubDirective));
 * const routerLinks = linkDes.map(de => de.injector.get(RouterLinkStubDirective));
 * expect(routerLinks.length).toBe(1, 'should have 1 routerLinks');
 * expect(routerLinks[0].linkParams).toBe('../');
 * testingUtility.sendClick(buttons[0]);
 * expect(routerLinks[0].navigatedTo).toBe('../');
 *
 */
@Directive({
  selector: '[routerLink]',
})
export class RouterLinkStubDirective {
  @Input('routerLink') linkParams: any;
  navigatedTo: any = null;

  @HostListener('click')
  onClick() {
    this.navigatedTo = this.linkParams;
  }
}
