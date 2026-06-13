package crypto

import (
	"bufio"
	"encoding/base64"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os/exec"
	"sync"
)

type Engine struct {
	cmd    *exec.Cmd
	stdin  io.WriteCloser
	reader *bufio.Reader
	mu     sync.Mutex
	keyHex string
}

type request struct {
	Op    string `json:"op"`
	Key   string `json:"key,omitempty"`
	Nonce string `json:"nonce,omitempty"`
	Data  string `json:"data,omitempty"`
}

type response struct {
	OK         bool   `json:"ok"`
	Error      string `json:"error,omitempty"`
	Key        string `json:"key,omitempty"`
	Nonce      string `json:"nonce,omitempty"`
	Ciphertext string `json:"ciphertext,omitempty"`
	Plaintext  string `json:"plaintext,omitempty"`
}

type EncryptedFile struct {
	Nonce      []byte
	Ciphertext []byte
}

func Start(binaryPath string) (*Engine, error) {
	cmd := exec.Command(binaryPath)

	stdin, err := cmd.StdinPipe()
	if err != nil {
		return nil, fmt.Errorf("open engine stdin: %w", err)
	}

	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return nil, fmt.Errorf("open engine stdout: %w", err)
	}

	if err := cmd.Start(); err != nil {
		return nil, fmt.Errorf("start engine: %w", err)
	}

	engine := &Engine{
		cmd:    cmd,
		stdin:  stdin,
		reader: bufio.NewReader(stdout),
	}

	keyHex, err := engine.GenerateKey()
	if err != nil {
		_ = engine.Close()
		return nil, err
	}
	engine.keyHex = keyHex

	return engine, nil
}

func (e *Engine) Close() error {
	if e.stdin != nil {
		_ = e.stdin.Close()
	}
	if e.cmd != nil && e.cmd.Process != nil {
		_ = e.cmd.Process.Kill()
		_, _ = e.cmd.Process.Wait()
	}
	return nil
}

func (e *Engine) GenerateKey() (string, error) {
	res, err := e.roundTrip(request{Op: "generate_key"})
	if err != nil {
		return "", err
	}
	if res.Key == "" {
		return "", errors.New("engine returned empty key")
	}
	return res.Key, nil
}

func (e *Engine) GenerateNonce() (string, error) {
	res, err := e.roundTrip(request{Op: "generate_nonce"})
	if err != nil {
		return "", err
	}
	if res.Nonce == "" {
		return "", errors.New("engine returned empty nonce")
	}
	return res.Nonce, nil
}

func (e *Engine) Encrypt(plaintext []byte) (EncryptedFile, error) {
	if e.keyHex == "" {
		return EncryptedFile{}, errors.New("engine key is not initialized")
	}

	nonceHex, err := e.GenerateNonce()
	if err != nil {
		return EncryptedFile{}, err
	}

	res, err := e.roundTrip(request{
		Op:    "encrypt",
		Key:   e.keyHex,
		Nonce: nonceHex,
		Data:  base64.StdEncoding.EncodeToString(plaintext),
	})
	if err != nil {
		return EncryptedFile{}, err
	}

	nonce, err := hex.DecodeString(nonceHex)
	if err != nil {
		return EncryptedFile{}, fmt.Errorf("decode nonce: %w", err)
	}

	ciphertext, err := base64.StdEncoding.DecodeString(res.Ciphertext)
	if err != nil {
		return EncryptedFile{}, fmt.Errorf("decode ciphertext: %w", err)
	}

	return EncryptedFile{Nonce: nonce, Ciphertext: ciphertext}, nil
}

func (e *Engine) roundTrip(req request) (response, error) {
	e.mu.Lock()
	defer e.mu.Unlock()

	payload, err := json.Marshal(req)
	if err != nil {
		return response{}, err
	}

	payload = append(payload, '\n')
	if _, err := e.stdin.Write(payload); err != nil {
		return response{}, fmt.Errorf("write engine request: %w", err)
	}

	line, err := e.reader.ReadBytes('\n')
	if err != nil {
		return response{}, fmt.Errorf("read engine response: %w", err)
	}

	var res response
	if err := json.Unmarshal(line, &res); err != nil {
		return response{}, fmt.Errorf("parse engine response: %w", err)
	}
	if !res.OK {
		if res.Error == "" {
			res.Error = "engine request failed"
		}
		return response{}, errors.New(res.Error)
	}

	return res, nil
}
