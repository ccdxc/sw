import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { LocalComponent } from './local.component';
import { By } from '@angular/platform-browser';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { AuthpolicybaseComponent } from '@app/components/settings-group/authpolicy/authpolicybase/authpolicybase.component';


describe('LocalComponent', () => {
  let component: LocalComponent;
  let fixture: ComponentFixture<LocalComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [LocalComponent],
      imports: [
        MaterialdesignModule, NoopAnimationsModule]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(LocalComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should display toggle based on input', () => {
    component.localData = { enabled: true };
    fixture.detectChanges();
    let toggle = fixture.debugElement.queryAll(By.css('.mat-checked'));
    expect(toggle.length).toBe(1);

    component.localData = { enabled: false };
    fixture.detectChanges();
    toggle = fixture.debugElement.queryAll(By.css('.mat-checked'));
    expect(toggle.length).toBe(0);
  });

  it('should display arrows and rank', () => {
    const spy = spyOn(component.changeAuthRank, 'emit');
    // Current rank isn't set, shouldn't show anything
    let rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
    expect(rank.length).toBe(0);

    // NO ARROWS
    component.currentRank = 0;
    component.numRanks = 1;
    fixture.detectChanges();
    let arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
    expect(arrow_container.length).toBe(1);
    expect(arrow_container[0].children.length).toBe(0);

    arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
    expect(arrow_container.length).toBe(1);
    expect(arrow_container[0].children.length).toBe(0);

    rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
    expect(rank.length).toBe(1);
    expect(rank[0].nativeElement.innerText).toContain('1');


    // UP ARROW ONLY
    component.currentRank = 1;
    component.numRanks = 2;
    fixture.detectChanges();
    arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
    expect(arrow_container.length).toBe(1);
    expect(arrow_container[0].children.length).toBe(0);

    arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
    expect(arrow_container.length).toBe(1);
    expect(arrow_container[0].children.length).toBe(1);
    arrow_container[0].children[0].nativeElement.click();
    expect(spy).toHaveBeenCalledTimes(1);
    expect(spy).toHaveBeenCalledWith(0);

    rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
    expect(rank.length).toBe(1);
    expect(rank[0].nativeElement.innerText).toContain('2');

    // DOWN ARROW ONLY
    spy.calls.reset();
    component.currentRank = 0;
    component.numRanks = 2;
    fixture.detectChanges();
    arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
    expect(arrow_container.length).toBe(1);
    expect(arrow_container[0].children.length).toBe(0);

    arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
    expect(arrow_container.length).toBe(1);
    expect(arrow_container[0].children.length).toBe(1);
    arrow_container[0].children[0].nativeElement.click();
    expect(spy).toHaveBeenCalledTimes(1);
    expect(spy).toHaveBeenCalledWith(1);

    rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
    expect(rank.length).toBe(1);
    expect(rank[0].nativeElement.innerText).toContain('1');

    // BOTH ARROWS
    component.currentRank = 1;
    component.numRanks = 3;
    fixture.detectChanges();
    arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
    expect(arrow_container.length).toBe(1);
    expect(arrow_container[0].children.length).toBe(1);

    arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
    expect(arrow_container.length).toBe(1);
    expect(arrow_container[0].children.length).toBe(1);

    rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
    expect(rank.length).toBe(1);
    expect(rank[0].nativeElement.innerText).toContain('2');
  });
});
