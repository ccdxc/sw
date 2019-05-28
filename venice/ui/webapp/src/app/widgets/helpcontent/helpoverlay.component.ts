import { Component, ViewChild, OnInit, OnDestroy, AfterViewInit  } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { OverlayRef, Overlay, GlobalPositionStrategy } from '@angular/cdk/overlay';
import { Subscription } from 'rxjs';
import { Portal, TemplatePortalDirective } from '@angular/cdk/portal';


@Component({
  selector: 'app-helpoverlay',
  templateUrl: './helpoverlay.component.html',
  styleUrls: ['./helpoverlay.component.scss']
})
export class HelpoverlayComponent implements OnInit, OnDestroy, AfterViewInit {
  subscriptions: Subscription[] = [];
  helpPortal: Portal<any>;
  @ViewChild('HelpShell') helpShell: TemplatePortalDirective;
  @ViewChild('DefaultHelp') defaultHelp: TemplatePortalDirective;

  helpOverlayRef: OverlayRef;

  constructor(protected controllerService: ControllerService,
    protected overlay: Overlay) {}

  ngOnInit() {
    this.subscriptions.push( this.controllerService.subscribe(Eventtypes.HELP_CONTENT_CHANGE, (payload) => {
      this.setHelpContent();
    }));

    this.subscriptions.push( this.controllerService.subscribe(Eventtypes.HELP_OVERLAY_CLOSE, (payload) => {
      this.closeHelp();
    }));

    this.subscriptions.push( this.controllerService.subscribe(Eventtypes.HELP_OVERLAY_OPEN, (payload) => {
      this.openHelp();
    }));
  }

  ngAfterViewInit() {
    this.helpOverlayRef = this.overlay.create({
      height: '400px',
      width: '400px',
      positionStrategy: new GlobalPositionStrategy().right('30px').top('100px'),
      panelClass: 'appcontent-help-panel'
    });
    this.setHelpContent();
  }

  setHelpContent() {
    let data = this.controllerService.getHelpOverlayData();
    if (data == null) {
      data = {};
    }
    if (data.component != null) {
      this.helpPortal = data.component;
    } else if (data.template != null) {
      this.helpPortal = data.template;
    } else {
      this.helpPortal = this.defaultHelp;
    }
  }

  toggleHelp($event) {
    if (!this.helpOverlayRef.hasAttached()) {
      this.helpOverlayRef.attach(this.helpShell);
    }  else {
      this.helpOverlayRef.detach();
    }
  }

  closeHelp() {
    if (this.helpOverlayRef.hasAttached()) {
      this.helpOverlayRef.detach();
    }
  }

  openHelp() {
    if (!this.helpOverlayRef.hasAttached()) {
      this.helpOverlayRef.attach(this.helpShell);
    }
  }

  ngOnDestroy() {
    this.subscriptions.forEach((s) => {
      s.unsubscribe();
    });
  }
}
