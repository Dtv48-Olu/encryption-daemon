package watcher

import (
	"fmt"
	"path/filepath"
	"strings"

	"github.com/fsnotify/fsnotify"
)

type Watcher struct {
	fs     *fsnotify.Watcher
	Events chan string
	Errors chan error
}

func New(paths []string) (*Watcher, error) {
	fsWatcher, err := fsnotify.NewWatcher()
	if err != nil {
		return nil, err
	}

	w := &Watcher{
		fs:     fsWatcher,
		Events: make(chan string, 128),
		Errors: make(chan error, 8),
	}

	for _, path := range paths {
		cleaned, err := filepath.Abs(path)
		if err != nil {
			_ = fsWatcher.Close()
			return nil, fmt.Errorf("resolve watch path %q: %w", path, err)
		}
		if err := fsWatcher.Add(cleaned); err != nil {
			_ = fsWatcher.Close()
			return nil, fmt.Errorf("watch %q: %w", cleaned, err)
		}
	}

	go w.loop()
	return w, nil
}

func (w *Watcher) Close() error {
	return w.fs.Close()
}

func (w *Watcher) loop() {
	defer close(w.Events)
	defer close(w.Errors)

	for {
		select {
		case event, ok := <-w.fs.Events:
			if !ok {
				return
			}
			if shouldProcess(event) {
				w.Events <- event.Name
			}
		case err, ok := <-w.fs.Errors:
			if !ok {
				return
			}
			w.Errors <- err
		}
	}
}

func shouldProcess(event fsnotify.Event) bool {
	if event.Name == "" {
		return false
	}
	if strings.HasSuffix(event.Name, ".enc") {
		return false
	}
	return event.Has(fsnotify.Create) || event.Has(fsnotify.Write) || event.Has(fsnotify.Rename)
}
